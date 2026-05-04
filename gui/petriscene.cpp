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

static void setNodeHighlight(QGraphicsItem *item, bool on) {
    if (auto *p = dynamic_cast<PlaceItem *>(item)) 
        p->setHighlighted(on);
    if (auto *t = dynamic_cast<TransitionItem *>(item))
        t->setHighlighted(on);
}

void PetriScene::setTool(Tool tool) {
    if (m_arcSource) {
        setNodeHighlight(m_arcSource, false);
        m_arcSource = nullptr;
    }
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
            QString name = QString("t%1").arg(++m_transitionCounter);
            transition->setName(name);
            addItem(transition);
            if (m_interp) {
                Transition *t = m_interp->createTransition(name.toStdString());
                transition->setInterpTransition(t);
            }
            log(QString("Přechod %1 přidán").arg(name));
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
                setNodeHighlight(m_arcSource, true);
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
        else if (auto *arc = dynamic_cast<ArcItem *>(clicked)) {
            showArcContextMenu(arc, event->screenPos());
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
        log(QString("%1: přidán jeden token").arg(place->name()));
    }
    else if (chosen == removeOne) {
        if (ip) ip->removeTokens(1);
        place->setTokens(ip ? (int)ip->getTokenCount() : place->tokens() - 1);
        log(QString("%1: odebrán jeden token").arg(place->name()));
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
        removeConnectedArcs(place);
        if (m_interp && place->interpPlace())
            m_interp->removePlace(place->interpPlace()->identifier);
        log(QString("Místo %1 smazáno").arg(place->name()));
        removeItem(place);
        delete place;
        emit selectionCleared();
    }

}

void PetriScene::showTransitionContextMenu(TransitionItem *transition, QPoint screenPos){
    QMenu menu;

    QAction *remove = menu.addAction("Odstranit přechod");

    QAction *chosen = menu.exec(screenPos);
    if (chosen == remove) {
        removeConnectedArcs(transition);
        if (m_interp && transition->interpTransition())
            m_interp->removeTransition(transition->interpTransition()->identifier);
        log(QString("Přechod %1 smazán").arg(transition->name()));
        removeItem(transition);
        delete transition;
        emit selectionCleared();
    }
}

void PetriScene::keyPressEvent(QKeyEvent *event){
    if (event->key() == Qt::Key_Delete){
        auto selected = selectedItems();
        if (selected.isEmpty())
            return;
        for (QGraphicsItem *item : selected){
            removeConnectedArcs(item);
            if (m_interp) {
                if (auto *place = dynamic_cast<PlaceItem *>(item))
                    if (place->interpPlace())
                        m_interp->removePlace(place->interpPlace()->identifier);
                if (auto *transition = dynamic_cast<TransitionItem *>(item))
                    if (transition->interpTransition())
                        m_interp->removeTransition(transition->interpTransition()->identifier);
            }
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

void PetriScene::removeConnectedArcs(QGraphicsItem *node)
{
    const QList<QGraphicsItem *> allItems = items();
    for (QGraphicsItem *item : allItems) {
        auto *arc = dynamic_cast<ArcItem *>(item);
        if (!arc || (arc->fromItem() != node && arc->toItem() != node))
            continue;

        auto *srcPlace = dynamic_cast<PlaceItem *>(arc->fromItem());
        auto *srcTrans = dynamic_cast<TransitionItem *>(arc->fromItem());
        auto *dstPlace = dynamic_cast<PlaceItem *>(arc->toItem());
        auto *dstTrans = dynamic_cast<TransitionItem *>(arc->toItem());

        if (srcPlace && dstTrans && srcPlace->interpPlace() && dstTrans->interpTransition())
            dstTrans->interpTransition()->removeEntryEdge(srcPlace->interpPlace());
        else if (srcTrans && dstPlace && srcTrans->interpTransition() && dstPlace->interpPlace())
            srcTrans->interpTransition()->removeExitEdge(dstPlace->interpPlace());

        auto nameOf = [](QGraphicsItem *item) -> QString {
            if (auto *p = dynamic_cast<PlaceItem *>(item)) 
                return p->name();
            if (auto *t = dynamic_cast<TransitionItem *>(item))
                return t->name();
        };
        
        log(QString("Hrana %1 → %2 smazána").arg(nameOf(arc->fromItem()), nameOf(arc->toItem())));
        removeItem(arc);
        delete arc;
    }
}

void PetriScene::showArcContextMenu(ArcItem *arc, QPoint screenPos)
{
    QMenu menu;
    QAction *remove = menu.addAction("Smazat hranu");

    if (menu.exec(screenPos) != remove)
        return;

    auto nameOf = [](QGraphicsItem *item) -> QString {
        if (auto *p = dynamic_cast<PlaceItem *>(item)) 
            return p->name();
        if (auto *t = dynamic_cast<TransitionItem *>(item))
            return t->name();
    };

    auto *srcPlace = dynamic_cast<PlaceItem *>(arc->fromItem());
    auto *srcTrans = dynamic_cast<TransitionItem *>(arc->fromItem());
    auto *dstPlace = dynamic_cast<PlaceItem *>(arc->toItem());
    auto *dstTrans = dynamic_cast<TransitionItem *>(arc->toItem());

    if (srcPlace && dstTrans && srcPlace->interpPlace() && dstTrans->interpTransition())
        dstTrans->interpTransition()->removeEntryEdge(srcPlace->interpPlace());

    else if (srcTrans && dstPlace && srcTrans->interpTransition() && dstPlace->interpPlace())
        srcTrans->interpTransition()->removeExitEdge(dstPlace->interpPlace());

    log(QString("Hrana %1 → %2 smazána").arg(nameOf(arc->fromItem()), nameOf(arc->toItem())));
    removeItem(arc);
    delete arc;
}

void PetriScene::drawArc(QGraphicsItem *target)
{
    setNodeHighlight(m_arcSource, false);
    ArcItem *arc = new ArcItem(m_arcSource, target);
    addItem(arc);

    if (m_interp) {
        auto *srcPlace = dynamic_cast<PlaceItem *>(m_arcSource);
        auto *srcTrans = dynamic_cast<TransitionItem *>(m_arcSource);
        auto *dstPlace = dynamic_cast<PlaceItem *>(target);
        auto *dstTrans = dynamic_cast<TransitionItem *>(target);

        if (srcPlace && dstTrans && srcPlace->interpPlace() && dstTrans->interpTransition())
            // Place → Transition: entry edge
            dstTrans->interpTransition()->addEntryEdge(srcPlace->interpPlace(), arc->weight());
        else if (srcTrans && dstPlace && srcTrans->interpTransition() && dstPlace->interpPlace())
            // Transition → Place: exit edge
            srcTrans->interpTransition()->addExitEdge(dstPlace->interpPlace(), arc->weight());
    }

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
    setNodeHighlight(m_arcSource, false);
    m_arcSource = nullptr;
}

void PetriScene::syncTokensFromInterpreter()
{
    for (QGraphicsItem *item : items()) {
        if (auto *place = dynamic_cast<PlaceItem *>(item))
            if (place->interpPlace())
                place->setTokens((int)place->interpPlace()->getTokenCount());
    }
}
