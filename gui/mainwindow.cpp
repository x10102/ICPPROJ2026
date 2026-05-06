/**
 * @file mainwindow.h
 * @author Dalibor Kalina, xkalin16
 * @brief Hlavní okno aplikace editoru Petriho sítí.
 */

#include "mainwindow.hpp"
#include "../petri.hpp"
#include "editorstate.hpp"
#include <QGraphicsView>
#include <QToolBar>
#include <QAction>
#include <QFont>
#include <QFrame>
#include <QDockWidget>
#include <QTabWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include <QLabel>
#include <QScrollBar>
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QToolButton>
#include <QTimer>
#include <QApplication>
#include <qaction.h>
#include <qmenu.h>
#include <qtoolbutton.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Fajný editorek");
    resize(1000,700); 

    m_scene = new PetriScene(this);
    m_scene->setNetworkSpec(&this->spec);

    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing);
    setCentralWidget(m_view);

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
        m_dock->show();
    });

    connect(m_scene, &PetriScene::transitionSelected, this, [this](TransitionItem *transition) {
        // Schovat vlastnosti místa a zobrazit vlastnosti přechodu
        m_editedPlace = nullptr;
        m_tokenSpin->setVisible(false);
        m_tokenLabel->setVisible(false);
        m_arcPanel->setVisible(true);
        m_nameEdit->setReadOnly(false);

        m_editedArc = nullptr;
        m_arcWeightPanel->setVisible(false);
        m_nameEdit->setReadOnly(false);

        m_editedTransition = transition;
        m_nameEdit->blockSignals(true);
        m_nameEdit->setText(transition->name());
        m_nameEdit->blockSignals(false);

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
}


// -------------------------
//          TERMINAL
// -------------------------
void MainWindow::setupTerminal() {
    m_terminalDock = new QDockWidget("Terminál", this);
    m_terminalDock->setAllowedAreas(Qt::BottomDockWidgetArea);

    QWidget *container = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(container);
    vbox->setContentsMargins(4, 4, 4, 4);
    vbox->setSpacing(4);

    QFont f("Monospace");
    f.setStyleHint(QFont::TypeWriter);
    f.setPointSize(9);

    auto makeLog = [&]() {
        auto *w = new QPlainTextEdit;
        w->setReadOnly(true);
        w->setMaximumBlockCount(500);
        w->setFont(f);
        w->setMinimumHeight(100);
        return w;
    };

    m_terminal  = makeLog();

    QTabWidget *tabs = new QTabWidget;
    tabs->addTab(m_terminal,  "GUI");
    vbox->addWidget(tabs);

    // Vstup pro příkazy uživatele
    QWidget *inputRow = new QWidget;
    QHBoxLayout *hbox = new QHBoxLayout(inputRow);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(4);

    m_terminalDock->setWidget(container);
    addDockWidget(Qt::BottomDockWidgetArea, m_terminalDock);
    m_terminalDock->hide();
}
 
void MainWindow::appendLog(const QString &msg) {
    m_terminal->appendPlainText(msg);
    m_terminal->verticalScrollBar()->setValue(m_terminal->verticalScrollBar()->maximum());
}
// -------------------------
//     end of TERMINAL
// -------------------------



// -------------------------
//          TOOLBAR
// -------------------------
void MainWindow::setupToolbar(){
    QToolBar *tb = addToolBar("Nástroje");
    tb->setMovable(false);

    // Create menus and menu open buttons    

    auto fileMenu = new QMenu(this);
    auto fileMenuButton = new QToolButton(this);
    fileMenuButton->setText("Soubor");
    fileMenuButton->setMenu(fileMenu);
    fileMenuButton->setPopupMode(QToolButton::InstantPopup);

    auto viewMenu = new QMenu(this);
    auto viewMenuButton = new QToolButton(this);
    viewMenuButton->setText("Zobrazit");
    viewMenuButton->setMenu(viewMenu);
    viewMenuButton->setPopupMode(QToolButton::InstantPopup);

    // Populate the submenus with buttons
    auto btnSave = fileMenu->addAction("Uložit síť");

    auto *termAct = viewMenu->addAction("Terminál");
    termAct->setCheckable(true);

    auto *darkAct = viewMenu->addAction("Dark Theme");
    darkAct->setCheckable(true);

    // Add the menus to the toolbar
    tb->addWidget(fileMenuButton);
    tb->addWidget(viewMenuButton);

    // Wire up the menu items to their actions
    connect(btnSave, &QAction::triggered, this, [this](){
        this->spec.exportJSON();
    });
    
    connect(termAct, &QAction::toggled, this, [this](bool checked) {
        checked ? m_terminalDock->show() : m_terminalDock->hide();
    });
    connect(m_terminalDock, &QDockWidget::visibilityChanged, termAct, &QAction::setChecked);
 
    connect(darkAct, &QAction::toggled, this, [this](bool checked) {
        if (checked)
            Theme::apply(Theme::dark());
        else 
            Theme::apply(Theme::light());
        this->applyTheme(Theme::current());
    });
}
// -------------------------
//     end of TOOLBAR
// -------------------------



// -------------------------
//     PANEL WITH BUTTONS
// -------------------------
void MainWindow::setupFloatingPanels() {
    // Panel nástrojů vlevo nahore
    m_toolPanel = new QFrame(m_view);
    m_toolPanel->setFrameShape(QFrame::StyledPanel);
    m_toolPanel->setStyleSheet(
        "QFrame { background: rgba(245,245,245,220); border-radius: 6px; }"
        "QPushButton { min-width: 90px; padding: 4px 8px; }"
        "QPushButton:checked { background: #4a90d9; color: white; border-radius: 3px; }"
    );

    QVBoxLayout *toolLayout = new QVBoxLayout(m_toolPanel);
    toolLayout->setContentsMargins(6,6,6,6);
    toolLayout->setSpacing(4);

    auto makeToolBtn = [&](const QString &label, Tool tool) {
        auto *btn = new QPushButton(label, m_toolPanel);
        btn->setCheckable(true);
        connect(btn, &QPushButton::clicked, this, [this, tool, btn]() {
            setActiveTool(tool, btn);
        });
        toolLayout->addWidget(btn);
        return btn;
    };

    QPushButton *panBtn    = makeToolBtn("Pan", Tool::Pan);
    QPushButton *selectBtn = makeToolBtn("Select", Tool::Select);
    makeToolBtn("Place", Tool::AddPlace);
    makeToolBtn("Transition", Tool::AddTransition);
    makeToolBtn("Arc", Tool::AddArc);
    makeToolBtn("Remove", Tool::Remove);

    m_toolPanel->adjustSize();
    m_toolPanel->move(10, 10);
    m_toolPanel->show();
    m_toolPanel->raise();

    (void)panBtn;
    setActiveTool(Tool::Select, selectBtn);

    // --- Panel simulace (vpravo dole) ---
    m_simPanel = new QFrame(m_view);
    m_simPanel->setFrameShape(QFrame::StyledPanel);
    m_simPanel->setStyleSheet(
        "QFrame { background: rgba(245,245,245,220); border-radius: 6px; }"
        "QPushButton { min-width: 60px; padding: 4px 12px; }"
        "QPushButton:checked { background: #4a90d9; color: white; border-radius: 3px; }"
    );

    QHBoxLayout *simLayout = new QHBoxLayout(m_simPanel);
    simLayout->setContentsMargins(6,6,6,6);
    simLayout->setSpacing(6);

    QPushButton *stepBtn = new QPushButton("Step", m_simPanel);
    stepBtn->setEnabled(false);
    simLayout->addWidget(stepBtn);

    m_runBtn = new QPushButton("Run", m_simPanel);
    m_runBtn->setCheckable(true);
    m_runBtn->setEnabled(false);
    simLayout->addWidget(m_runBtn);

    m_simPanel->adjustSize();
    m_simPanel->show();
    m_simPanel->raise();
    QTimer::singleShot(0, this, [this]() { repositionSimPanel(); });
}


void MainWindow::repositionSimPanel() {
    if (!m_simPanel || !m_view) return;
    QRect vp = m_view->viewport()->geometry();
    m_simPanel->move(vp.right()  - m_simPanel->width()  - 10,
                     vp.bottom() - m_simPanel->height() - 10);
    m_simPanel->raise();
    m_toolPanel->raise();
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
// -------------------------
//     end of PANEL WITH BUTTONS
// -------------------------



// -------------------------
//          SIDEBAR
// -------------------------
void MainWindow::setupSidebar(){
    m_dock = new QDockWidget("Vlastnosti", this);
    m_dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    m_dock->setMinimumWidth(200);

    QWidget *panel = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(panel);
    vbox->setContentsMargins(10,10,10,10);
    vbox->setSpacing(8);

    // Sdílené vlastnosti
    QFormLayout *form = new QFormLayout;
    m_nameEdit = new QLineEdit;
    form->addRow("Jméno: ", m_nameEdit);
    vbox->addLayout(form);

    // Vlastnosti místa
    QFormLayout *tokenForm = new QFormLayout;
    m_tokenSpin = new QSpinBox;
    m_tokenSpin->setMinimum(0);
    m_tokenSpin->setMaximum(9999);
    m_tokenLabel = new QLabel("Tokeny:");
    tokenForm->addRow(m_tokenLabel, m_tokenSpin);
    vbox->addLayout(tokenForm);

    // Vlastnosti přechodu
    m_arcPanel = new QWidget;
    m_arcLayout = new QVBoxLayout(m_arcPanel);
    m_arcLayout->setContentsMargins(0,0,0,0);
    m_arcLayout->setSpacing(4);
    vbox->addWidget(m_arcPanel);
    m_arcPanel->setVisible(false);

    m_arcWeightPanel = new QWidget;
    QFormLayout *wf = new QFormLayout(m_arcWeightPanel);
    m_arcWeightLabel = new QLabel("Váha:");
    m_arcWeightSpin  = new QSpinBox;
    m_arcWeightSpin->setMinimum(1);
    m_arcWeightSpin->setMaximum(9999);
    wf->addRow(m_arcWeightLabel, m_arcWeightSpin);
    vbox->addWidget(m_arcWeightPanel);
    m_arcWeightPanel->setVisible(false);

    vbox->addStretch();

    connect(m_nameEdit, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_editedPlace) {
            m_editedPlace->setName(text);
        }
        if (m_editedTransition) {
            m_editedTransition->setName(text);
        }
    });
    connect(m_tokenSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val){
        if (m_editedPlace) {
            m_editedPlace->setTokens(val);
        }
    });
    connect(m_arcWeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val) {
        if (m_editedArc) m_editedArc->setWeight(val);
    });

    m_dock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, m_dock);
    m_dock->hide();
}
// -------------------------
//     end of SIDEBAR
// -------------------------

void MainWindow::clearArcRows()
{
    QLayoutItem *item;
    while ((item = m_arcLayout->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
}

void MainWindow::populateTransitionSidebar(TransitionItem *transition){
    clearArcRows();

    QList<ArcItem *> incoming, outgoing;
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto *arc = dynamic_cast<ArcItem *>(item)) {
            if (arc->toItem()   == transition)
                incoming.append(arc);
            if (arc->fromItem() == transition)
                outgoing.append(arc);
        }
    }

    if (incoming.isEmpty() && outgoing.isEmpty()) {
        m_arcLayout->addWidget(new QLabel("Žádné příchozí nebo odchozí hrany"));
    }

    if (!incoming.isEmpty()){
        m_arcLayout->addWidget(new QLabel("<br>Příchozí<br>"));

        for (ArcItem *arc : incoming){
            auto *place = dynamic_cast<PlaceItem *>(arc->fromItem());
            QString placeName = place ? place->name() : "?";

            QWidget *row = new QWidget;
            QHBoxLayout *hbox = new QHBoxLayout(row);
            hbox->setContentsMargins(0, 0, 0, 0);
            hbox->addWidget(new QLabel(QString("%1 ->").arg(placeName)));

            QSpinBox *wspin = new QSpinBox;
            wspin->setMinimum(0);
            wspin->setMaximum(9999);
            wspin->setValue(arc->weight());
            wspin->setFixedWidth(60);
            hbox->addWidget(wspin);

            connect(wspin, QOverload<int>::of(&QSpinBox::valueChanged), this, [arc, transition, place](int val) {
                arc->setWeight(val);
            });

            m_arcLayout->addWidget(row);
        }
    }
    if (!outgoing.isEmpty()){
        m_arcLayout->addWidget(new QLabel("<br>Odchozí<br>"));

        for (ArcItem *arc : outgoing){
            auto *place = dynamic_cast<PlaceItem *>(arc->toItem());
            QString placeName = place ? place->name() : "?";

            QWidget *row = new QWidget;
            QHBoxLayout *hbox = new QHBoxLayout(row);
            hbox->setContentsMargins(0, 0, 0, 0);
            hbox->addWidget(new QLabel(QString("-> %1").arg(placeName)));

            QSpinBox *wspin = new QSpinBox;
            wspin->setMinimum(0);
            wspin->setMaximum(9999);
            wspin->setValue(arc->weight());
            wspin->setFixedWidth(60);
            hbox->addWidget(wspin);

            connect(wspin, QOverload<int>::of(&QSpinBox::valueChanged), this, [arc, transition, place](int val) {
                arc->setWeight(val);
            });

            m_arcLayout->addWidget(row);
        }
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

    m_toolPanel->setStyleSheet(QString(
        "QFrame { background: %1; border-radius: 6px; }"
        "QPushButton { color: %2; min-width: 90px; padding: 4px 8px; }"
        "QPushButton:checked { background: #4a90d9; color: white; border-radius: 3px; }"
    ).arg(background, text));

    m_simPanel->setStyleSheet(QString(
        "QFrame { background: %1; border-radius: 6px; }"
        "QPushButton { color: %2; min-width: 60px; padding: 4px 12px; }"
        "QPushButton:checked { background: #4a90d9; color: white; border-radius: 3px; }"
    ).arg(background, text));
}