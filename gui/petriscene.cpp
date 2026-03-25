#include "petriscene.h"
#include "../petri.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QKeyEvent>
#include <QDateTime>

PetriScene::PetriScene(QObject *parent) : QGraphicsScene(parent) {
    setSceneRect(0,0,SCENE_W,SCENE_H);
}

void PetriScene::setTool(Tool tool) {
    m_tool = tool;
}

void PetriScene::setInterpreter(QtInterpreter *interp) {
    m_interp = interp;
}

void PetriScene::log(const QString &msg) {
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    emit logMessage(QString("[%1] %2").arg(time, msg));
}

void PetriScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if (event->button() == Qt::LeftButton) {
        QPointF pos = event->scenePos();

        switch (m_tool) {
        case Tool::AddPlace: {
            auto *place = new PlaceItem(pos);
            QString name = QString("p%1").arg(++m_placeCounter);
            place->setName(name);
            addItem(place);
            if (m_interp) {
                Place *p = m_interp->createPlace(name.toStdString(), place->tokens());
                place->setInterpPlace(p);
            }
            log(QString("Místo %1 přidáno").arg(name));
            break;
        }
        case Tool::AddTransition: {
            auto *transition = new TransitionItem(pos);
            addItem(transition);
            log(QString("Přechod %1 přidán").arg(transition->name()));
            break;
        }
        case Tool::AddArc: {
            QGraphicsItem *clicked = itemAt(pos, QTransform());

            bool isNode = (dynamic_cast<PlaceItem *>(clicked) || dynamic_cast<TransitionItem *>(clicked));
            if (!isNode) {
                cancelArc();
                break;
            }

            if (!m_arcSource){
                m_arcSource = clicked;
            }
            else {
                if (clicked != m_arcSource){
                    drawArc(clicked);
                }
                else {
                    cancelArc();
                }
            }
            break;
        }
        case Tool::Select:
        default:
            if (event->modifiers() & Qt::ControlModifier)
                event->setModifiers(Qt::NoModifier);

            QGraphicsScene::mousePressEvent(event);

            if (auto *place = dynamic_cast<PlaceItem *>(mouseGrabberItem()))
                emit placeSelected(place);
            else if (selectedItems().isEmpty())
                emit selectionCleared();
            else if (auto *place = dynamic_cast<PlaceItem *>(selectedItems().first()))
                emit placeSelected(place);
            else if (auto *transition = dynamic_cast<TransitionItem *>(selectedItems().first()))
                emit transitionSelected(transition);
            else
                emit selectionCleared();
            break;
        }
    }
    else if (event->button() == Qt::RightButton){
        QGraphicsItem *clicked = itemAt(event->scenePos(), QTransform());
        if (auto *place = dynamic_cast<PlaceItem *>(clicked)) {
            showPlaceContextMenu(place, event->screenPos());
        }
        else if (auto *transition = dynamic_cast<TransitionItem *>(clicked)) {
            showTransitionContextMenu(transition, event->screenPos());
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
    QAction *setZero = menu.addAction("Vynulovat tokeny");
    QAction *reset = menu.addAction("Resetovat tokeny");
    menu.addSeparator();
    QAction *remove = menu.addAction("Smazat místo");

    removeOne->setEnabled(place->tokens() > 0);
    setZero->setEnabled(place->tokens() > 0);

    QAction *chosen = menu.exec(screenPos);
    Place *ip = place->interpPlace();
    if (chosen == addOne) {
        if (ip) ip->addTokens(1);
        place->setTokens(ip ? (int)ip->getTokenCount() : place->tokens() + 1);
    }
    else if (chosen == removeOne) {
        if (ip) ip->removeTokens(1);
        place->setTokens(ip ? (int)ip->getTokenCount() : place->tokens() - 1);
    }
    else if (chosen == setZero) {
        if (ip) ip->removeTokens(ip->getTokenCount());
        place->setTokens(0);
        log(QString("%1: tokeny vynulovány").arg(place->name()));
    }
    else if (chosen == reset) {
        if (ip) {
            uint32_t cur = ip->getTokenCount();
            uint32_t init = ip->getInitTokens();
            if (init > cur) ip->addTokens(init - cur);
            else if (cur > init) ip->removeTokens(cur - init);
            place->setTokens((int)ip->getTokenCount());
        }
        log(QString("%1: tokeny resetovány").arg(place->name()));
    }
    else if (chosen == remove) {
        log(QString("Místo %1 smazáno").arg(place->name()));
        removeItem(place);
        delete place;
        emit selectionCleared();
    }

}

void PetriScene::showTransitionContextMenu(TransitionItem *transition, QPoint screenPos){
    QMenu menu;

    QAction *temp = menu.addAction("Random temporary thingie lol");

    QAction *chosen = menu.exec(screenPos);
    if (chosen == temp) {
        return;
    }
}

void PetriScene::keyPressEvent(QKeyEvent *event){
    if (event->key() == Qt::Key_Delete){
        auto selected = selectedItems();
        if (selected.isEmpty())
            return;
        for (QGraphicsItem *item : selected){
            removeItem(item);
            delete item;
        }
        return;
    }
    QGraphicsScene::keyPressEvent(event);
}

void PetriScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    for (QGraphicsItem *item : items()) {
        if (auto *arc = dynamic_cast<ArcItem *>(item)) {
            arc->updatePosition();
        }
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void PetriScene::drawArc(QGraphicsItem *target)
{
    addItem(new ArcItem(m_arcSource, target));

    auto nameOf = [](QGraphicsItem *item) -> QString {
        if (auto *p = dynamic_cast<PlaceItem *>(item))      return p->name();
        if (auto *t = dynamic_cast<TransitionItem *>(item)) return t->name();
        return "?";
    };
    log(QString("Hrana přidána: %1 → %2").arg(nameOf(m_arcSource), nameOf(target)));

    if (auto *t = dynamic_cast<TransitionItem *>(m_arcSource))
        emit transitionSelected(t);
    else if (auto *t = dynamic_cast<TransitionItem *>(target))
        emit transitionSelected(t);

    m_arcSource = nullptr;
}

void PetriScene::cancelArc()
{
    m_arcSource = nullptr;
}
