/**
 * @file mw_toolbars.cpp
 * @author Ondřej Turek, xtureko00, Dalibor Kalina, xkalin16
 * @brief GUI Tool widgets implementation
 */
#include "geninterp.hpp"
#include "gui/udpconnector.hpp"
#include "mainwindow.hpp"
#include "variableeditor.hpp"
#include "styles.hpp"
#include <QFormLayout>
#include <QWidget>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QToolBar>
#include <QMenu>
#include <QToolButton>
#include <QTimer>
#include <QAction>
#include <QLineEdit>
#include <QPushButton>
#include <qpushbutton.h>

void MainWindow::setupSidebar(){
    // Attribute sidebar on the right side of the window
    m_dock = new QDockWidget("Vlastnosti", this);
    m_dock->setFeatures(QDockWidget::DockWidgetClosable);
    m_dock->setMinimumWidth(200);
    QWidget *panel = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(panel);
    vbox->setContentsMargins(10,10,10,10);
    vbox->setSpacing(8);

    // --- shared attributes ---

    // Name of the selected element
    QFormLayout *form = new QFormLayout;
    m_nameEdit = new QLineEdit;
    form->addRow("Jméno: ", m_nameEdit);
    vbox->addLayout(form);

    // Action of the selected element
    QFormLayout *actionForm = new QFormLayout;
    m_actionEdit = new QLineEdit;
    actionForm->addRow(new QLabel("Akce:"));
    actionForm->addRow(m_actionEdit);
    vbox->addLayout(actionForm);

    
    // --- place-specific attributes ---

    // Token count of the place
    QFormLayout *tokenForm = new QFormLayout;
    m_tokenSpin = new QSpinBox;
    m_tokenSpin->setMinimum(0);
    m_tokenSpin->setMaximum(9999);
    m_tokenLabel = new QLabel("Tokeny:");
    tokenForm->addRow(m_tokenLabel, m_tokenSpin);
    vbox->addLayout(tokenForm);


    // --- transition-specific attributes ---

    // Firing condition of the transition
    QFormLayout *fireCondForm = new QFormLayout;
    m_fireCondEdit = new QLineEdit;
    m_fireCondLabel = new QLabel("Podmínka odpálení:");
    fireCondForm->addRow(m_fireCondLabel);
    fireCondForm->addRow(m_fireCondEdit);
    vbox->addLayout(fireCondForm);

    // Input event of the transition
    QFormLayout *inputEvtNameForm = new QFormLayout;
    m_evtNameEdit = new QLineEdit;
    m_evtNameLabel = new QLabel("Název vstupní události:");
    inputEvtNameForm->addRow(m_evtNameLabel);
    inputEvtNameForm->addRow(m_evtNameEdit);
    vbox->addLayout(inputEvtNameForm);


    // --- arc-specific attributes ---
    m_arcPanel = new QWidget;
    m_arcLayout = new QVBoxLayout(m_arcPanel);
    m_arcLayout->setContentsMargins(0,0,0,0);
    m_arcLayout->setSpacing(4);
    vbox->addWidget(m_arcPanel);
    m_arcPanel->setVisible(false);

    // Setting the weight of the arc
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

    // --- wiring up the attribute editors to update the Petri net specification ---
    connect(m_nameEdit, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_editedPlace) {
            m_spec.renamePlace(m_editedPlace->name().toStdString(), text.toStdString());
            m_editedPlace->setName(text);
        }
        if (m_editedTransition) {
            m_spec.renameTransition(m_editedTransition->name().toStdString(), text.toStdString());
            m_editedTransition->setName(text);
        }
    });
    connect(m_tokenSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val){
        if (m_editedPlace) {
            m_editedPlace->setTokens(val);
            m_spec.getPlace(m_editedPlace->name().toStdString())->initial_tokens = val;
        }
    });
    connect(m_arcWeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val) {
        if (m_editedArc) {
            m_editedArc->setWeight(val);
            m_spec.getArc(m_editedArc)->tokenCount = val;
        }
    });
    connect(m_actionEdit, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_editedPlace) {
            m_editedPlace->setAction(text);
            m_spec.getPlace(m_editedPlace->name().toStdString())->placeActionMacro = text.toStdString();
        }
        if (m_editedTransition) {
            m_editedTransition->setAction(text);
            m_spec.getTransition(m_editedTransition->name().toStdString())->tranActionMacro = text.toStdString();
        }
    });
    connect(m_fireCondEdit, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_editedTransition) {
            m_editedTransition->setFireCond(text);
            m_spec.getTransition(m_editedTransition->name().toStdString())->booleanGuardMacro = text.toStdString();
        }
    });
    connect(m_evtNameEdit, &QLineEdit::textEdited, this, [this](const QString &text){
        if (m_editedTransition) {
            m_editedTransition->setInputEvtName(text);
            m_spec.getTransition(m_editedTransition->name().toStdString())->inputEventName = text.toStdString();
        }
    });


    m_dock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, m_dock);
    m_dock->hide();
}



void MainWindow::setupToolbar(){
    // Create setup toolbar with dropdown menus at the top of the window
    QToolBar *tb = addToolBar("Nástroje");
    tb->setMovable(false);

    // -- Dropdown menus ---
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

    auto netMenu = new QMenu(this);
    auto netMenuButton = new QToolButton(this);
    netMenuButton->setText("Síť");
    netMenuButton->setMenu(netMenu);
    netMenuButton->setPopupMode(QToolButton::InstantPopup);


    // -- Menu items for each dropdown menu ---
    auto btnSave = fileMenu->addAction("Uložit síť");
    auto btnLoad = fileMenu->addAction("Načíst síť");
    auto btnSetSource = fileMenu->addAction("Vybrat zdrojový adresář");

    auto nameAct = netMenu->addAction("Vlastnosti");
    auto variablesAct = netMenu->addAction("Proměnné");
    auto compileAct = netMenu->addAction("Sestavit interpret");
    auto runAct = netMenu->addAction("Spustit interpret");

    auto termAct = viewMenu->addAction("Terminál");
    termAct->setCheckable(true);
    auto darkAct = viewMenu->addAction("Dark Theme");
    darkAct->setCheckable(true);

    // Add the menus to the toolbar
    tb->addWidget(fileMenuButton);
    tb->addWidget(netMenuButton);
    tb->addWidget(viewMenuButton);


    // -- Connecting menu items to their respective actions ---
    connect(btnSave, &QAction::triggered, this, [this](){
        if (this->saveNet()) {
            this->m_spec.exportJSON();
        }
    });

    connect(btnLoad, &QAction::triggered, this, [this](){
        this->loadNet();
    });

    connect(variablesAct, &QAction::triggered, this, [this](){
        showVariableEditor(this->variables, this);
    });

    connect(btnSetSource, &QAction::triggered, this, [this](){this->setSourceDir();});
    
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

    connect(compileAct, &QAction::triggered, this, [this](){
        compileInterpreter();
    });

    connect(nameAct, &QAction::triggered, this, [this](){
        this->openNetPropsDialog();
    });
}

void MainWindow::setupFloatingPanels() {
    // Tool panel with buttons for selecting the active tool
    m_toolPanel = new QFrame(m_view);
    m_toolPanel->setFrameShape(QFrame::StyledPanel);
    m_toolPanel->setStyleSheet(styles::ToolPanelDefault);

    QVBoxLayout *toolLayout = new QVBoxLayout(m_toolPanel);
    toolLayout->setContentsMargins(6,6,6,6);
    toolLayout->setSpacing(4);

    // Lambda function to create a tool button and connect it to setActiveTool
    auto makeToolBtn = [&](const QString &label, Tool tool) {
        auto *btn = new QPushButton(label, m_toolPanel);
        btn->setCheckable(true);
        connect(btn, &QPushButton::clicked, this, [this, tool, btn]() {
            setActiveTool(tool, btn);
        });
        toolLayout->addWidget(btn);
        return btn;
    };

    // Create the buttons for each tool
    makeToolBtn("Posun", Tool::Pan);
    QPushButton *selectBtn = makeToolBtn("Výběr", Tool::Select);
    makeToolBtn("Místo", Tool::AddPlace);
    makeToolBtn("Přechod", Tool::AddTransition);
    makeToolBtn("Hrana", Tool::AddArc);
    makeToolBtn("Odstranit", Tool::Remove);

    m_toolPanel->adjustSize();
    m_toolPanel->move(10, 10);
    m_toolPanel->show();
    m_toolPanel->raise();

    // Set the default active tool to Select
    setActiveTool(Tool::Select, selectBtn);



    // -- Simulation panel with buttons for controlling the simulation in the lower right corner
    m_simPanel = new QFrame(m_view);
    m_simPanel->setFrameShape(QFrame::StyledPanel);
    m_simPanel->setStyleSheet(styles::SimPanelDefault);

    QHBoxLayout *simLayout = new QHBoxLayout(m_simPanel);
    simLayout->setContentsMargins(6,6,6,6);
    simLayout->setSpacing(6);

    m_stepBtn = new QPushButton("Krok", m_simPanel);
    m_stepBtn->setEnabled(false);
    connect(m_stepBtn, &QPushButton::clicked, this, [this](){this->m_receiver->sendStep(true);});

    m_contBtn = new QPushButton("Pokračovat", m_simPanel);
    m_contBtn->setEnabled(false);
    connect(m_contBtn, &QPushButton::clicked, this->m_receiver, &UdpConnector::sendStep);
    
    m_runBtn = new QPushButton("Spustit", m_simPanel);
    m_runBtn->setEnabled(false);
    connect(m_runBtn, &QPushButton::clicked, this->m_generator, &InterpreterGenerator::run);

    simLayout->addWidget(m_runBtn);
    simLayout->addWidget(m_stepBtn);
    simLayout->addWidget(m_contBtn);

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



void MainWindow::setupTerminal() {
    // Terminal dock at the bottom of the window
    m_terminalDock = new QDockWidget("Terminál", this);
    m_terminalDock->setAllowedAreas(Qt::BottomDockWidgetArea);

    QWidget *container = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(container);
    vbox->setContentsMargins(4, 4, 4, 4);
    vbox->setSpacing(4);

    QFont f("Monospace");
    f.setStyleHint(QFont::TypeWriter);
    f.setPointSize(9);

    // Lambda function to create a read-only log widget
    auto makeLog = [&]() {
        auto *w = new QPlainTextEdit;
        w->setReadOnly(true);
        w->setMaximumBlockCount(500);
        w->setFont(f);
        w->setMinimumHeight(100);
        return w;
    };

    m_terminal  = makeLog();
    m_buildTerminal = makeLog();
    m_interpTerminal = makeLog();

    m_terminalTabs = new QTabWidget;
    m_terminalTabs->addTab(m_terminal,  "GUI");
    m_terminalTabs->addTab(m_buildTerminal, "Sestavení");
    m_terminalTabs->addTab(m_interpTerminal, "Interpret");
    vbox->addWidget(m_terminalTabs);

    // User input row with a line edit and send button
    QWidget *inputRow = new QWidget;
    QHBoxLayout *hbox = new QHBoxLayout(inputRow);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(4);

    m_terminalInput = new QLineEdit;
    m_terminalInput->setFont(f);
    m_terminalInput->setPlaceholderText("Příkaz");
    hbox->addWidget(m_terminalInput);

    QPushButton *sendBtn = new QPushButton("Odeslat");
    hbox->addWidget(sendBtn);

    vbox->addWidget(inputRow);

    // Connecting the send button and Enter key to send commands from the input line edit
    connect(sendBtn, &QPushButton::clicked, this, [this](){
        QString cmd = m_terminalInput->text().trimmed();
        if (cmd.isEmpty())
            return;
        m_terminalInput->clear();
        
        // Tohle nahradit. Aktualne printuju jen do GUI terminalu
        appendLog(QString("> %1").arg(cmd));      
    });

    connect(m_terminalInput, &QLineEdit::returnPressed, sendBtn, &QPushButton::click);

    m_terminalDock->setWidget(container);
    addDockWidget(Qt::BottomDockWidgetArea, m_terminalDock);
}



void MainWindow::populateTransitionSidebar(TransitionItem *transition){
    // Clear previous arc weight editors
    QLayoutItem *arcItem;
    while ((arcItem = m_arcLayout->takeAt(0)) != nullptr) {
        if (arcItem->widget())
            arcItem->widget()->deleteLater();
        delete arcItem;
    }

    // Finding all arcs connected to the transition
    QList<ArcItem *> incoming, outgoing;
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto *arc = qgraphicsitem_cast<ArcItem *>(item)) {
            if (arc->toItem() == transition)
                incoming.append(arc);
            if (arc->fromItem() == transition)
                outgoing.append(arc);
        }
    }

    if (incoming.isEmpty() && outgoing.isEmpty()) {
        m_arcLayout->addWidget(new QLabel("Žádné příchozí nebo odchozí hrany"));
    }

    // List of incoming arcs with editable weights
    if (!incoming.isEmpty()){
        m_arcLayout->addWidget(new QLabel("<br>Příchozí<br>"));

        for (ArcItem *arc : incoming){
            auto *place = qgraphicsitem_cast<PlaceItem *>(arc->fromItem());
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

            connect(wspin, QOverload<int>::of(&QSpinBox::valueChanged), this, [arc, transition, place, this](int val) {
                arc->setWeight(val);
                this->m_spec.getArc(arc)->tokenCount = val;
            });

            m_arcLayout->addWidget(row);
        }
    }

    // List of outgoing arcs with editable weights
    if (!outgoing.isEmpty()){
        m_arcLayout->addWidget(new QLabel("<br>Odchozí<br>"));

        for (ArcItem *arc : outgoing){
            auto *place = qgraphicsitem_cast<PlaceItem *>(arc->toItem());
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

            connect(wspin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, arc, transition, place](int val) {
                arc->setWeight(val);
                this->m_spec.getArc(arc)->tokenCount = val;
            });

            m_arcLayout->addWidget(row);
        }
    }
}