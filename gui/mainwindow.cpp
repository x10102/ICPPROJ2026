#include "mainwindow.h"
#include <QGraphicsView>
#include <QToolBar>
#include <QAction>
#include <QFont>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Fajný editorek");
    resize(1000,700);

    m_scene = new PetriScene(this);
    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    setCentralWidget(m_view);

    setupToolbar();
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

void MainWindow::setActiveTool(Tool tool, QAction *action) {
    if (m_activeAction) m_activeAction->setChecked(false);
    m_activeAction = action;
    if (m_activeAction) m_activeAction->setChecked(true);

    m_scene->setTool(tool);

    m_view->setDragMode(tool==Tool::Select ? QGraphicsView::ScrollHandDrag : QGraphicsView::NoDrag);
}
