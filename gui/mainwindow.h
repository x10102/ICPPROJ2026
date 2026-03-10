#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "petriscene.h"

class QGraphicsView;
class QAction;

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupToolbar();
    void setActiveTool(Tool tool, QAction *action);

    PetriScene *m_scene = nullptr;
    QGraphicsView *m_view = nullptr;
    QAction *m_activeAction = nullptr;
};

#endif // MAINWINDOW_H
