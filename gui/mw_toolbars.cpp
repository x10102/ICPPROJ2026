/**
 * @file mw_toolbars.cpp
 * @author Ondřej Turek, xtureko00, Dalibor Kalina, xkalin16
 * @brief Implementace nástrojových widgetů v GUI
 */
#include "gui/udpconnector.hpp"
#include "mainwindow.hpp"
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

void MainWindow::setupSidebar(){
    m_dock = new QDockWidget("Vlastnosti", this);
    m_dock->setFeatures(QDockWidget::DockWidgetClosable);
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

    QFormLayout *actionForm = new QFormLayout;
    m_actionEdit = new QLineEdit;
    actionForm->addRow(new QLabel("Akce:"));
    actionForm->addRow(m_actionEdit);
    vbox->addLayout(actionForm);
    
    // Vlastnosti místa
    QFormLayout *tokenForm = new QFormLayout;
    m_tokenSpin = new QSpinBox;
    m_tokenSpin->setMinimum(0);
    m_tokenSpin->setMaximum(9999);
    m_tokenLabel = new QLabel("Tokeny:");
    tokenForm->addRow(m_tokenLabel, m_tokenSpin);
    vbox->addLayout(tokenForm);


    // Vlastnosti přechodu
    QFormLayout *fireCondForm = new QFormLayout;
    m_fireCondEdit = new QLineEdit;
    m_fireCondLabel = new QLabel("Podmínka odpálení:");
    fireCondForm->addRow(m_fireCondLabel);
    fireCondForm->addRow(m_fireCondEdit);
    vbox->addLayout(fireCondForm);

    QFormLayout *inputEvtNameForm = new QFormLayout;
    m_evtNameEdit = new QLineEdit;
    m_evtNameLabel = new QLabel("Název vstupní události:");
    inputEvtNameForm->addRow(m_evtNameLabel);
    inputEvtNameForm->addRow(m_evtNameEdit);
    vbox->addLayout(inputEvtNameForm);

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

    auto netMenu = new QMenu(this);
    auto netMenuButton = new QToolButton(this);
    netMenuButton->setText("Síť");
    netMenuButton->setMenu(netMenu);
    netMenuButton->setPopupMode(QToolButton::InstantPopup);

    // Populate the submenus with buttons
    auto btnSave = fileMenu->addAction("Uložit síť");
    auto btnLoad = fileMenu->addAction("Načíst síť");
    auto btnSetSource = fileMenu->addAction("Vybrat zdrojový adresář");

    auto termAct = viewMenu->addAction("Terminál");
    termAct->setCheckable(true);

    auto darkAct = viewMenu->addAction("Dark Theme");
    darkAct->setCheckable(true);

    auto nameAct = netMenu->addAction("Vlastnosti");
    auto compileAct = netMenu->addAction("Sestavit interpret");
    auto runAct = netMenu->addAction("Spustit interpret");

    // Add the menus to the toolbar
    tb->addWidget(fileMenuButton);
    tb->addWidget(netMenuButton);
    tb->addWidget(viewMenuButton);

    // Wire up the menu items to their actions
    connect(btnSave, &QAction::triggered, this, [this](){
        if (this->saveNet()) {
            this->m_spec.exportJSON();
        }
    });

    connect(btnLoad, &QAction::triggered, this, [this](){
        this->loadNet();
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
    // Panel nástrojů vlevo nahore
    m_toolPanel = new QFrame(m_view);
    m_toolPanel->setFrameShape(QFrame::StyledPanel);
    m_toolPanel->setStyleSheet(styles::ToolPanelDefault);

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
    m_simPanel->setStyleSheet(styles::SimPanelDefault);

    QHBoxLayout *simLayout = new QHBoxLayout(m_simPanel);
    simLayout->setContentsMargins(6,6,6,6);
    simLayout->setSpacing(6);

    QPushButton *stepBtn = new QPushButton("Step", m_simPanel);
    stepBtn->setEnabled(true);
    simLayout->addWidget(stepBtn);

    connect(stepBtn, &QPushButton::clicked, this->m_receiver, &UdpConnector::sendStep);

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
    m_build_terminal = makeLog();

    QTabWidget *tabs = new QTabWidget;
    tabs->addTab(m_terminal,  "GUI");
    tabs->addTab(m_build_terminal, "Sestavení");
    vbox->addWidget(tabs);

    // Vstup pro příkazy uživatele
    QWidget *inputRow = new QWidget;
    QHBoxLayout *hbox = new QHBoxLayout(inputRow);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(4);

    m_terminalInput = new QLineEdit;
    m_terminalInput->setFont(f);
    m_terminalInput->setPlaceholderText("Prikazy sem...");
    hbox->addWidget(m_terminalInput);

    QPushButton *sendBtn = new QPushButton("Send");
    hbox->addWidget(sendBtn);

    vbox->addWidget(inputRow);

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
    //m_terminalDock->hide();
}

void MainWindow::populateTransitionSidebar(TransitionItem *transition){
    QLayoutItem *arcItem;
    while ((arcItem = m_arcLayout->takeAt(0)) != nullptr) {
        if (arcItem->widget())
            arcItem->widget()->deleteLater();
        delete arcItem;
    }

    QList<ArcItem *> incoming, outgoing;
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto *arc = qgraphicsitem_cast<ArcItem *>(item)) {
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