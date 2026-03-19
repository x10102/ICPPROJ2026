#include "mainwindow.h"
#include <QGraphicsView>
#include <QToolBar>
#include <QAction>
#include <QFont>
#include <QDockWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include <QLabel>
#include <QScrollBar>
#include <QPlainTextEdit>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Fajný editorek");
    resize(1000,700);

    m_scene = new PetriScene(this);
    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    setCentralWidget(m_view);

    setupToolbar();
    setupSidebar();
    setupTerminal();
 
    connect(m_scene, &PetriScene::logMessage, this, &MainWindow::appendLog);

    connect(m_scene, &PetriScene::placeSelected, this, [this](PlaceItem *place) {
        // Schovat vlastnosti přechodu a zobrazit vlastnosti místa
        m_editedTransition = nullptr;
        m_tokenSpin->setVisible(true);
        m_tokenLabel->setVisible(true);
        m_arcPanel->setVisible(false);

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

        m_editedTransition = transition;
        m_nameEdit->blockSignals(true);
        m_nameEdit->setText(transition->name());
        m_nameEdit->blockSignals(false);

        populateTransitionSidebar(transition);

        m_dock->show();
    });

    connect(m_scene, &PetriScene::selectionCleared, this, [this](){
        m_editedPlace = nullptr;
        m_editedTransition = nullptr;
        m_dock->hide();
    });
}

void MainWindow::setupTerminal() {
    m_terminalDock = new QDockWidget("Terminál", this);
    m_terminalDock->setAllowedAreas(Qt::BottomDockWidgetArea);
 
    m_terminal = new QPlainTextEdit;
    m_terminal->setReadOnly(true);
    m_terminal->setMaximumBlockCount(500);
    QFont f("Monospace");
    f.setStyleHint(QFont::TypeWriter);
    f.setPointSize(9);
    m_terminal->setFont(f);
    m_terminal->setMinimumHeight(100);
 
    m_terminalDock->setWidget(m_terminal);
    addDockWidget(Qt::BottomDockWidgetArea, m_terminalDock);
    m_terminalDock->hide();
 
    // Toggle button in toolbar — added after toolbar is set up
    // so we just add it here via a stored toolbar reference
}
 
void MainWindow::appendLog(const QString &msg) {
    m_terminal->appendPlainText(msg);
    // Auto-scroll to bottom
    m_terminal->verticalScrollBar()->setValue(m_terminal->verticalScrollBar()->maximum());
}

void MainWindow::setupToolbar(){
    QToolBar *tb = addToolBar("Nástroje");
    tb->setMovable(false);

    QFont f = tb->font();
    f.setPointSize(10);
    tb->setFont(f);

    auto makeAction = [&](const QString &label, Tool tool) {
        QAction *a = tb->addAction(label);
        a->setCheckable(true);
        connect(a, &QAction::triggered, this, [this, tool, a]() {
            setActiveTool(tool, a);
        });
        return a;
    };

    QAction *selectAct = makeAction("Select", Tool::Select);
    QAction *placeAct = makeAction("Place", Tool::AddPlace);
    QAction *transitionAct = makeAction("Transition", Tool::AddTransition);
    QAction *ArcAct = makeAction("Arc", Tool::AddArc);

    setActiveTool(Tool::Select, selectAct);

    tb->addSeparator();
    QAction *termAct = tb->addAction("Terminál");
    termAct->setCheckable(true);
    connect(termAct, &QAction::toggled, this, [this](bool checked) {
        checked ? m_terminalDock->show() : m_terminalDock->hide();
    });
    connect(m_terminalDock, &QDockWidget::visibilityChanged, termAct, &QAction::setChecked);

    (void)placeAct;
    (void)transitionAct;
}

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

    vbox->addStretch();

    connect(m_nameEdit, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_editedPlace) m_editedPlace->setName(text);
        if (m_editedTransition) m_editedTransition->setName(text);
    });
    connect(m_tokenSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val){
        if (m_editedPlace) m_editedPlace->setTokens(val);
    });

    m_dock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, m_dock);
    m_dock->hide();
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
        QLabel *header = new QLabel("<br>Příchozí<br>");
        m_arcLayout->addWidget(header);

        for (ArcItem *arc : incoming){
            auto *place = dynamic_cast<PlaceItem *>(arc->fromItem());
            QString placeName = place ? place->name() : "?";

            QWidget *row = new QWidget;
            QHBoxLayout *hbox = new QHBoxLayout(row);
            hbox->setContentsMargins(0, 0, 0, 0);
            QString label = QString("%1 ->").arg(placeName);
            hbox->addWidget(new QLabel(label));

            QSpinBox *wspin = new QSpinBox;
            wspin->setMinimum(0);
            wspin->setMaximum(9999);
            wspin->setValue(arc->weight());
            wspin->setFixedWidth(60);
            hbox->addWidget(wspin);

            connect(wspin, QOverload<int>::of(&QSpinBox::valueChanged), this, [arc](int val) {
                arc->setWeight(val);
            });

            m_arcLayout->addWidget(row);
        }
    }
    if (!outgoing.isEmpty()){
        QLabel *header = new QLabel("<br>Odchozí<br>");
        m_arcLayout->addWidget(header);

        for (ArcItem *arc : outgoing){
            auto *place = dynamic_cast<PlaceItem *>(arc->toItem());
            QString placeName = place ? place->name() : "?";

            QWidget *row = new QWidget;
            QHBoxLayout *hbox = new QHBoxLayout(row);
            hbox->setContentsMargins(0, 0, 0, 0);
            QString label = QString("-> %1").arg(placeName);
            hbox->addWidget(new QLabel(label));

            QSpinBox *wspin = new QSpinBox;
            wspin->setMinimum(0);
            wspin->setMaximum(9999);
            wspin->setValue(arc->weight());
            wspin->setFixedWidth(60);
            hbox->addWidget(wspin);

            connect(wspin, QOverload<int>::of(&QSpinBox::valueChanged), this, [arc](int val) {
                arc->setWeight(val);
            });

            m_arcLayout->addWidget(row);
        }
    }
}

void MainWindow::setActiveTool(Tool tool, QAction *action) {
    if (m_activeAction) m_activeAction->setChecked(false);
    m_activeAction = action;
    if (m_activeAction) m_activeAction->setChecked(true);

    m_scene->setTool(tool);

    m_view->setDragMode(tool==Tool::Select ? QGraphicsView::ScrollHandDrag : QGraphicsView::NoDrag);
}
