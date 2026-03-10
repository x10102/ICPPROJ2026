#include "petriscene.h"
#include <QGraphicsSceneMouseEvent>

PetriScene::PetriScene(QObject *parent) : QGraphicsScene(parent) {
    setSceneRect(0,0,2000,2000);
}

void PetriScene::setTool(Tool tool) {
    m_tool = tool;
}

void PetriScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if (event->button() != Qt::LeftButton) {
        return;
    }

    QPointF pos = event->scenePos();

    switch (m_tool) {
    case Tool::AddPlace:
        addItem(new PlaceItem(pos));
        break;
    case Tool::AddTransition:
        addItem(new TransitionItem(pos));
        break;
    //case Tool::AddArc:

    case Tool::Select:
    default:
        QGraphicsScene::mousePressEvent(event);
        break;
    }
}

void PetriScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    // TODO nejaky update na arcs

    QGraphicsScene::mouseMoveEvent(event);
}
