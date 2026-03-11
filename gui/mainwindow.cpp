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

    connect(m_scene, &PetriScene::placeSelected, this, [this](PlaceItem *place) {
        m_editedPlace = place;
        m_nameEdit->blockSignals(true);
        m_tokenSpin->blockSignals(true);
        m_nameEdit->setText(place->name());
        m_tokenSpin->setValue(place->tokens());
        m_nameEdit->blockSignals(false);
        m_tokenSpin->blockSignals(false);
        m_dock->show();
    });

    connect(m_scene, &PetriScene::selectionCleared, this, [this](){
        m_editedPlace = nullptr;
        m_dock->hide();
    });
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

    (void)placeAct;
    (void)transitionAct;
}

void MainWindow::setupSidebar(){
    m_dock = new QDockWidget("Vlastnosti", this);
    m_dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    m_dock->setMinimumWidth(200);

    QWidget *panel = new QWidget;
    QFormLayout *form = new QFormLayout(panel);
    form->setContentsMargins(10,10,10,10);
    form->setSpacing(8);

    m_nameEdit = new QLineEdit;
    m_tokenSpin = new QSpinBox;
    m_tokenSpin->setMinimum(0);
    m_tokenSpin->setMaximum(9999);

    form->addRow("Jméno: ", m_nameEdit);
    form->addRow("Tokeny: ", m_tokenSpin);

    connect(m_nameEdit, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_editedPlace) m_editedPlace->setName(text);
    });
    connect(m_tokenSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val){
        if (m_editedPlace) m_editedPlace->setTokens(val);
    });

    m_dock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, m_dock);
    m_dock->hide();
}

void MainWindow::setActiveTool(Tool tool, QAction *action) {
    if (m_activeAction) m_activeAction->setChecked(false);
    m_activeAction = action;
    if (m_activeAction) m_activeAction->setChecked(true);

    m_scene->setTool(tool);

    m_view->setDragMode(tool==Tool::Select ? QGraphicsView::ScrollHandDrag : QGraphicsView::NoDrag);
}
