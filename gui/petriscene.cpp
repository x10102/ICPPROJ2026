#include "petriscene.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

PetriScene::PetriScene(QObject *parent) : QGraphicsScene(parent) {
    setSceneRect(0,0,2000,2000);
}

void PetriScene::setTool(Tool tool) {
    m_tool = tool;
}

void PetriScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if (event->button() == Qt::LeftButton) {
        QPointF pos = event->scenePos();

        switch (m_tool) {
        case Tool::AddPlace:
            addItem(new PlaceItem(pos));
            break;
        case Tool::AddTransition:
            addItem(new TransitionItem(pos));
            break;
        case Tool::AddArc:
            //TODO fuck, what to do here?
            // Nejak si pamatovat dve kliknuti a spojit je i guess
            break;

        case Tool::Select:
        default:
            QGraphicsScene::mousePressEvent(event);
            break;
        }
    }
    else if (event->button() == Qt::RightButton){
        QGraphicsItem *clicked = itemAt(event->scenePos(), QTransform());
        if (auto *place = dynamic_cast<PlaceItem *>(clicked)) {
            showPlaceContextMenu(place, event->screenPos());
        }
    }
    else {
        return;
    }
}

void PetriScene::showPlaceContextMenu(PlaceItem *place, QPoint screenPos){
    QMenu menu;

    QAction *addOne = menu.addAction("Přidej token");
    QAction *removeOne = menu.addAction("Oddělej token");
    QAction *setZero = menu.addAction("TODO - vynulovat tokeny");
    QAction *reset = menu.addAction("TODO - resetovat tokeny");

    removeOne->setEnabled(place->tokens() > 0);
    setZero->setEnabled(place->tokens() > 0);

    QAction *chosen = menu.exec(screenPos);
    if (chosen == addOne) { place->addToken();}
    else if (chosen == removeOne) { place->removeToken();}
    else if (chosen == setZero) { } //TODO
    else if (chosen == reset) { } //TODO

}

void PetriScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    // TODO nejaky update na arcs

    QGraphicsScene::mouseMoveEvent(event);
}
