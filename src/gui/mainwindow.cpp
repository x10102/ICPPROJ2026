/**
 * @file mainwindow.h
 * @author Dalibor Kalina, xkalin16, Ondřej Turek, xtureko00
 * @brief Main application window of the Petri network editor
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
#include <map>
#include <qchar.h>
#include <qfiledialog.h>
#include <qglobal.h>
#include <qgraphicsitem.h>
#include <qmenu.h>
#include <qpoint.h>
#include <qstringliteral.h>
#include <qthread.h>
#include <QDialog>
#include <QDialogButtonBox>
#include <QThread>
#include <sstream>
#include <string>

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

    m_spec.setVariableMap(&this->variables);

    setupSourceGenerator();
    setupUDPThread();

    setupTerminal();
    setupToolbar();
    setupSidebar();
    setupFloatingPanels();

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
        m_evtNameEdit->setVisible(false);
        m_evtNameLabel->setVisible(false);
        m_actionEdit->setVisible(true);
        m_actionLabel->setVisible(true);

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
        m_evtNameEdit->setVisible(true);
        m_evtNameLabel->setVisible(true);
        m_actionEdit->setVisible(true);
        m_actionLabel->setVisible(true);

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
        m_evtNameEdit->setVisible(false);
        m_evtNameLabel->setVisible(false);
        m_actionEdit->setVisible(false);
        m_actionLabel->setVisible(false);

        m_arcWeightPanel->setVisible(true);
        
        m_editedArc = arc;
        m_nameEdit->blockSignals(true);

        QString fromName = dynamic_cast<INamed*>(arc->fromItem())->name();
        QString toName = dynamic_cast<INamed*>(arc->toItem())->name();

        m_nameEdit->setText(QString("hrana z %1 -> %2").arg(fromName, toName));
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

    
}

void MainWindow::setupSourceGenerator() {
    m_generator = new InterpreterGenerator(this);
    m_generator->setMarker("#### MARKER ####");
    connect(m_generator, &InterpreterGenerator::compileStarted, this, [this]() {
        m_terminalTabs->setCurrentIndex(TerminalTab::BUILD);
        appendLog("Kompiluji interpret...", TerminalTab::BUILD);
    });
    connect(m_generator, &InterpreterGenerator::compileProgress, this, [this](QString msg) {
        appendLog(msg, TerminalTab::BUILD);
    });
    connect(m_generator, &InterpreterGenerator::compileFinished, this, [this]() {
        appendLog("Interpret úspěšně sestaven.", TerminalTab::BUILD);
        m_interpRunnable = true;
        m_runBtn->setEnabled(true);
    });
    connect(m_generator, &InterpreterGenerator::compileFailed, this, [this]() {
        m_terminalTabs->setCurrentIndex(TerminalTab::BUILD);
        appendLog("Kompilace selhala. Zkontrolujte syntaxi akcí míst/přechodů.", TerminalTab::BUILD);
        m_interpRunnable = false;
        m_runBtn->setEnabled(false);
    });
    connect(m_generator, &InterpreterGenerator::interpreterStarted, this, [this]() {
        m_terminalTabs->setCurrentIndex(TerminalTab::INTERPRETER);
        appendLog("Interpret spuštěn.", TerminalTab::INTERPRETER);
        m_interpRunning = true;
        m_stepBtn->setEnabled(true);
        m_runBtn->setEnabled(false);
        m_contBtn->setEnabled(true);
    });
    connect(m_generator, &InterpreterGenerator::interpreterOutput, this, [this](QString line) {
        appendLog(line, TerminalTab::INTERPRETER);
    });
    connect(m_generator, &InterpreterGenerator::interpreterStopped, this, [this](int exitCode) {
        appendLog(QString("Interpret ukončen (kód %1).").arg(exitCode), TerminalTab::INTERPRETER);
        m_interpRunning = false;
        m_stepBtn->setEnabled(false);
        m_contBtn->setEnabled(false);
        m_runBtn->setEnabled(true);
    });
    connect(m_generator, &InterpreterGenerator::interpreterError, this, [this](QString reason) {
        appendLog("Chyba interpretu: " + reason, TerminalTab::INTERPRETER);
        m_interpRunning = false;
        m_stepBtn->setEnabled(false);
        m_contBtn->setEnabled(false);
        m_runBtn->setEnabled(true);
    });

}

void MainWindow::setupUDPThread() {
    m_receiver = new UdpConnector(this, 6767);
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
            selectedTab = m_buildTerminal;
            break;
        case TerminalTab::INTERPRETER:
            selectedTab = m_interpTerminal;
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

void MainWindow::openVariablesDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Definovat proměnné");
    dialog.setFixedSize(400, 300);

    QFormLayout form(&dialog);
    auto descEdit = new QPlainTextEdit(QString::fromStdString(this->m_spec.description), &dialog);
    descEdit->setMinimumHeight(100);
    form.addRow("Definice:", descEdit);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
    form.addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted){
        m_spec.setDescription(descEdit->toPlainText().toStdString());
    }
}

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
    QFileDialog fd;
    fd.setDefaultSuffix("pnet");
    QString filename = fd.getSaveFileName(this, "Uložit síť", QString::fromStdString(m_spec.name), "Petri Net specification (*.pnet)");
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

bool MainWindow::loadNet() {
    QString filename = QFileDialog::getOpenFileName(this, "Načíst síť", QString::fromStdString(m_spec.name), "Petri Net specification (*.pnet)");
    if (filename.isEmpty()) {
        appendLog("Výběr zrušen");
        return false;
    }

    QString givenName = QFileInfo(filename).baseName();
    if (!givenName.isEmpty()) {
        m_spec.setNetworkName(givenName.toStdString());
    }
    QFile outputFile(filename);

    if(!outputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        appendLog("Chyba: Nelze otevřít soubor " + filename + " pro čtení.");
    }

    m_spec.loadJSON(outputFile.readAll().toStdString());

    // Create two maps for the transition and place items we create
    // So that we don't have to inefficiently search m_scene.items() on every arc added
    std::map<std::string, PlaceItem*> createdPlaces;
    std::map<std::string, TransitionItem*> createdTransitions;

    for(const auto &[_, data] : m_spec.places) {
        QPointF position(data.x, data.y);
        auto *p = new PlaceItem(position);
        p->setName(QString::fromStdString(data.name));
        p->setAction(QString::fromStdString(data.placeActionMacro));
        p->setTokens(data.initial_tokens);
        m_scene->addItem(p);
        createdPlaces[data.name] = p;
    }

    for(const auto &[_, data] : m_spec.transitions) {
        QPointF position(data.x, data.y);
        auto *t = new TransitionItem(position);
        t->setName(QString::fromStdString(data.name));
        t->setAction(QString::fromStdString(data.tranActionMacro));
        t->setFireCond(QString::fromStdString(data.booleanGuardMacro));
        t->setInputEvtName(QString::fromStdString(data.inputEventName));
        m_scene->addItem(t);
        createdTransitions[data.name] = t;
    }

    for(const auto &arc : m_spec.arcs) {
        const PetriArc &data = arc.second;
        const std::string placeName = data.place->name;
        const std::string transitionName = data.transition->name;
        ArcItem *a;
        if(data.type == PLACE_TO_TRANSITION) {
            a = new ArcItem(createdPlaces[placeName], createdTransitions[transitionName]);
        } else {
            a = new ArcItem(createdTransitions[transitionName], createdPlaces[placeName]);
        }
        a->setWeight(data.tokenCount);
        m_scene->addItem(a);
    }
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
    //std::cout << picojson::value(data) << std::endl;
}
