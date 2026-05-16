/**
 * @file mainwindow.h
 * @author Dalibor Kalina, xkalin16, Ondřej Turek, xtureko00
 * @brief Hlavní okno aplikace editoru Petriho sítí.
 */

#include "mainwindow.hpp"
#include "editorstate.hpp"
#include "geninterp.hpp"
#include "gui/petriscene.hpp"
#include "gui/picojson.h"
#include "gui/udpconnector.hpp"
#include "items.hpp"
#include "styles.hpp"
#include "terminaltab.hpp"
#include <QGraphicsView>
#include <QToolBar>
#include <QAction>
#include <QFont>
#include <QFrame>
#include <QFormLayout>
#include <QFileDialog>
#include <QScrollBar>
#include <QTimer>
#include <QApplication>
#include <cstdlib>
#include <filesystem>
#include <qglobal.h>
#include <qmenu.h>
#include <qthread.h>
#include <QDialog>
#include <QDialogButtonBox>
#include <QThread>
#include <sstream>

MainWindow::~MainWindow() {
    if(m_receiverThread) {
        m_receiverThread->quit();
        m_receiverThread->wait();
    }
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Twinium");
    resize(1000,700); 

    m_scene = new PetriScene(this);
    m_scene->setNetworkSpec(&this->m_spec);

    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing);
    setCentralWidget(m_view);

    setupTerminal();
    setupToolbar();
    setupSidebar();
    setupFloatingPanels();

    // TODO: Doesn't work, fix or delete
    //qInstallMessageHandler(&MainWindow::terminalMessageHandler);

    m_view->installEventFilter(this);
    m_view->viewport()->installEventFilter(this);

    connect(m_scene, &PetriScene::logMessage, this, &MainWindow::appendLog);

    connect(m_scene, &PetriScene::placeSelected, this, [this](PlaceItem *place) {
        // Schovat vlastnosti přechodu a zobrazit vlastnosti místa
        m_editedTransition = nullptr;
        m_tokenSpin->setVisible(true);
        m_tokenLabel->setVisible(true);
        m_arcPanel->setVisible(false);
        m_nameEdit->setReadOnly(false);
        m_fireCondEdit->setVisible(false);
        m_fireCondLabel->setVisible(false);

        m_editedArc = nullptr;
        m_arcWeightPanel->setVisible(false);
        m_nameEdit->setReadOnly(false);

        m_editedPlace = place;
        m_nameEdit->blockSignals(true);
        m_tokenSpin->blockSignals(true);
        m_nameEdit->setText(place->name());
        m_tokenSpin->setValue(place->tokens());
        m_nameEdit->blockSignals(false);
        m_tokenSpin->blockSignals(false);
        
        m_actionEdit->blockSignals(true);
        m_actionEdit->setText(place->action());
        m_actionEdit->blockSignals(false);

        m_dock->show();
    });

    connect(m_scene, &PetriScene::transitionSelected, this, [this](TransitionItem *transition) {
        // Schovat vlastnosti místa a zobrazit vlastnosti přechodu
        m_editedPlace = nullptr;
        m_tokenSpin->setVisible(false);
        m_tokenLabel->setVisible(false);
        m_arcPanel->setVisible(true);
        m_nameEdit->setReadOnly(false);
        m_fireCondEdit->setVisible(true);
        m_fireCondLabel->setVisible(true);

        m_editedArc = nullptr;
        m_arcWeightPanel->setVisible(false);
        m_nameEdit->setReadOnly(false);

        m_editedTransition = transition;
        m_nameEdit->blockSignals(true);
        m_nameEdit->setText(transition->name());
        m_nameEdit->blockSignals(false);

        m_actionEdit->blockSignals(true);
        m_actionEdit->setText(transition->action());
        m_actionEdit->blockSignals(false);

        m_fireCondEdit->blockSignals(true);
        m_fireCondEdit->setText(transition->fireCond());
        m_fireCondEdit->blockSignals(false);

        populateTransitionSidebar(transition);

        m_dock->show();
    });

    connect(m_scene, &PetriScene::arcSelected, this, [this](ArcItem *arc) {        
        m_editedPlace = nullptr;
        m_editedTransition = nullptr;
        m_tokenSpin->setVisible(false);
        m_tokenLabel->setVisible(false);
        m_arcPanel->setVisible(false);

        m_arcWeightPanel->setVisible(true);
        
        m_editedArc = arc;
        m_nameEdit->blockSignals(true);

        auto nameOf = [](QGraphicsItem *item) -> QString {
            if (auto *p = dynamic_cast<PlaceItem *>(item)) {
                return p->name();
            }
            if (auto *t = dynamic_cast<TransitionItem *>(item)){
                return t->name();
            }
            return "?";
        };

        m_nameEdit->setText(QString("hrana z %1 -> %2").arg(nameOf(arc->fromItem()), nameOf(arc->toItem())));
        m_nameEdit->setReadOnly(true);
        m_nameEdit->blockSignals(false);
        m_arcWeightSpin->blockSignals(true);
        m_arcWeightSpin->setValue(arc->weight());
        m_arcWeightSpin->blockSignals(false);
        m_dock->show();

    });

    connect(m_scene, &PetriScene::selectionCleared, this, [this](){
        m_editedPlace = nullptr;
        m_editedTransition = nullptr;
        m_editedArc = nullptr;
        m_dock->hide();
    });

    setupSourceGenerator();
    setupUDPThread();
}

void MainWindow::setupSourceGenerator() {
    m_generator = new InterpreterGenerator(this);
    m_generator->setMarker("#### MARKER ####");
    m_generator->setPath("/mnt/c/users/cracktek/desktop/zdrojovy_kod_velkeho_mleka/ICPHOVNO");
    connect(m_generator, &InterpreterGenerator::compileStarted, this, [this]() {
    appendLog("Kompiluji interpret...", TerminalTab::BUILD);
    });
    connect(m_generator, &InterpreterGenerator::compileProgress, this, [this](QString msg) {
        appendLog(msg, TerminalTab::BUILD);
    });
    connect(m_generator, &InterpreterGenerator::compileFinished, this, [this]() {
        appendLog("Interpret úspěšně sestaven.", TerminalTab::BUILD);
    });
    connect(m_generator, &InterpreterGenerator::compileFailed, this, [this]() {
        appendLog("Kompilace selhala. Zkontrolujte syntaxi akcí míst/přechodů.", TerminalTab::BUILD);
    });
}

void MainWindow::setupUDPThread() {
    m_receiver = new UdpConnector(this, 6768);
    m_receiverThread = new QThread();
    m_receiverThread->moveToThread(m_receiverThread);

    connect(m_receiverThread, &QThread::started, m_receiver, &UdpConnector::start);
    connect(m_receiverThread, &QThread::finished, m_receiver, &QObject::deleteLater);
    connect(m_receiver, &UdpConnector::dataReceived, this, &MainWindow::onDataReceived);
    connect(m_receiver, &UdpConnector::dataReceived, m_scene, &PetriScene::onDataReceived);

    m_receiverThread->start();
}

void MainWindow::appendLog(const QString &msg, const TerminalTab tab) {
    QPlainTextEdit *selectedTab;
    switch(tab) {
        case TerminalTab::BUILD:
            selectedTab = m_build_terminal;
            break;
        default:
        case TerminalTab::GUI:
            selectedTab = m_terminal;
            break;
    }
    selectedTab->appendPlainText(msg);
    selectedTab->verticalScrollBar()->setValue(selectedTab->verticalScrollBar()->maximum());
}

void MainWindow::compileInterpreter() {
     appendLog("Generuji kód interpretu...", TerminalTab::BUILD);
    if (!m_generator->generateMain(&m_spec)) {
        appendLog("Generování selhalo.", TerminalTab::BUILD);
        return;
    }
    m_generator->compile();
}

void MainWindow::openNetPropsDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Změnit název a popis sítě");
    dialog.setFixedSize(400, 300);

    QFormLayout form(&dialog);
    auto nameEdit = new QLineEdit(QString::fromStdString(this->m_spec.name), &dialog);
    form.addRow("Název:", nameEdit);
    auto descEdit = new QPlainTextEdit(QString::fromStdString(this->m_spec.description), &dialog);
    descEdit->setMinimumHeight(100);
    form.addRow("Popis:", descEdit);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
    form.addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted){
        m_spec.setNetworkName(nameEdit->text().toStdString());
        m_spec.setDescription(descEdit->toPlainText().toStdString());
    }
}

// -------------------------
//     PANEL WITH BUTTONS
// -------------------------


bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_view && event->type() == QEvent::Resize) {
        QTimer::singleShot(0, this, [this]() { repositionSimPanel(); });
        return false;
    }

    if (obj == m_view->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::MiddleButton) {
                m_panLastPos = me->pos();
                m_isPanning  = true;
                m_view->viewport()->setCursor(Qt::ClosedHandCursor);
                return true;
            }
        } else if (event->type() == QEvent::MouseMove && m_isPanning) {
            auto *me = static_cast<QMouseEvent *>(event);
            QPoint delta = me->pos() - m_panLastPos;
            m_panLastPos = me->pos();
            m_view->horizontalScrollBar()->setValue(
                m_view->horizontalScrollBar()->value() - delta.x());
            m_view->verticalScrollBar()->setValue(
                m_view->verticalScrollBar()->value() - delta.y());
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            auto *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::MiddleButton) {
                m_isPanning = false;
                m_view->viewport()->unsetCursor();
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::clearArcRows()
{
    QLayoutItem *item;
    while ((item = m_arcLayout->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
}

void MainWindow::setActiveTool(Tool tool, QPushButton *btn) {
    if (m_activeToolBtn)
        m_activeToolBtn->setChecked(false);

    m_activeToolBtn = btn;

    if (m_activeToolBtn)
        m_activeToolBtn->setChecked(true);

    m_scene->setTool(tool);

    switch (tool) {
    case Tool::Pan:
        m_view->setDragMode(QGraphicsView::ScrollHandDrag);
        break;
    case Tool::Select:
        m_view->setDragMode(QGraphicsView::RubberBandDrag);
        break;
    default:
        m_view->setDragMode(QGraphicsView::NoDrag);
        break;
    }
}

bool MainWindow::setSourceDir() {
    QString dir = QFileDialog::getExistingDirectory(this, "Vybrat zdrojovou složku");
    if(dir.isEmpty()) {
        appendLog("Výběr zrušen");
        return false;
    }
    std::filesystem::path path = dir.toStdString();
    if(!std::filesystem::exists(path / MAIN_FILENAME)) {
        appendLog("Složka neobsahuje program.cpp, vybrali jste správně?");
        return false;
    }
    m_generator->setPath(path);
    appendLog("Používám zdrojový kód z " + dir, TerminalTab::BUILD);
    return true;
}

bool MainWindow::saveNet() {
    QString filename = QFileDialog::getSaveFileName(this, "Uložit síť", QString::fromStdString(m_spec.name), "Petri Net specification (*.pnet)");
    if (filename.isEmpty()) {
        appendLog("Výběr zrušen");
        return false;
    }

    QString givenName = QFileInfo(filename).baseName();
    if (!givenName.isEmpty()) {
        m_spec.setNetworkName(givenName.toStdString());
    }
    QFile outputFile(filename);

    if(!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        appendLog("Chyba: Nelze otevřít soubor " + filename + " pro zápis.");
    }

    outputFile.write(m_spec.exportJSON().c_str());
    appendLog("Specifikace sítě uložena do " + filename);
    return true;
}

void MainWindow::applyTheme(const Theme &theme){
    QPalette p;
    p.setColor(QPalette::Window, theme.windowBackground);
    p.setColor(QPalette::WindowText, theme.windowText);
    p.setColor(QPalette::Base, theme.windowBackground);
    p.setColor(QPalette::Text, theme.windowText);
    p.setColor(QPalette::Button, theme.windowBackground);
    p.setColor(QPalette::ButtonText, theme.windowText);
    
    qApp->setPalette(p);

    m_scene->applyTheme(theme);

    QString background = theme.windowBackground.name();
    QString text = theme.windowText.name();

    m_toolPanel->setStyleSheet(styles::ToolPanelCustom.arg(background, text));
    m_simPanel->setStyleSheet(styles::SimPanelCustom.arg(background, text));
}

void MainWindow::onDataReceived(picojson::object &data) {
    std::cout << picojson::value(data) << std::endl;
}