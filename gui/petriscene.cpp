/**
 * @file petriscene.cpp
 * @author Dalibor Kalina, xkalin16
 * @brief PetriNet Scene editor
 */

#include "petriscene.hpp"
#include "editorstate.hpp"
#include "gui/picojson.h"
#include "items.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QKeyEvent>
#include <QDateTime>
#include <cstdint>
#include <qgraphicsitem.h>

using picojson::value;
using picojson::object;
using std::string;

PetriScene::PetriScene(QObject *parent) : QGraphicsScene(parent) {
    setSceneRect(0,0,SCENE_W,SCENE_H);
}

static bool isNode(QGraphicsItem *n) {
    if(!n) return false;
    return n->type() == PlaceItem::Type || n->type() == TransitionItem::Type; 
}

static void setNodeHighlight(QGraphicsItem *item, bool on) {
    if (auto *p = qgraphicsitem_cast<PlaceItem *>(item)) 
        p->setHighlighted(on);
    if (auto *t = qgraphicsitem_cast<TransitionItem *>(item))
        t->setHighlighted(on);
}

static QString nameOf(QGraphicsItem *item) {
    if (auto *n = dynamic_cast<INamed*>(item))
        return n->name();
    return "?";
}

void PetriScene::setNetworkSpec(PetriNetworkSpec *spec) {
    this->spec = spec;
}

void PetriScene::setTool(Tool tool) {
    if (m_arcSource) {
        setNodeHighlight(m_arcSource, false);
        m_arcSource = nullptr;
    }
    m_tool = tool;
}

void PetriScene::log(const QString &msg) {
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    emit logMessage(QString("[%1] %2").arg(time, msg), GUI);
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
            PetriPlace storeT;
            storeT.name = name.toStdString();
            storeT.x = pos.x();
            storeT.y = pos.y();
            storeT.initial_tokens = 1;
            this->spec->addPlace(storeT);
            log(QString("Místo %1 přidáno").arg(name));
            break;
        }

        case Tool::AddTransition: {
            auto *transition = new TransitionItem(pos);
            QString name = QString("t%1").arg(++m_transitionCounter);
            transition->setName(name);
            addItem(transition);
            PetriTransition storeT;
            storeT.name = name.toStdString();
            storeT.x = pos.x();
            storeT.y = pos.y();
            this->spec->addTransition(storeT);
            log(QString("Přechod %1 přidán").arg(name));
            break;
        }

        case Tool::AddArc: {
            QGraphicsItem *clicked = itemAt(pos, QTransform());
            if (!isNode(clicked)) {
                cancelArc();
                break;
            }

            if (!isNode(m_arcSource)){
                m_arcSource = clicked;
                setNodeHighlight(m_arcSource, true);
                break;
            }

            if (clicked == m_arcSource) {
                cancelArc();
                break;
            }
            drawArc(clicked);
            string fromName = dynamic_cast<INamed*>(m_arcSource)->name().toStdString();
            string toName = dynamic_cast<INamed*>(clicked)->name().toStdString();
            if(clicked->type() == PlaceItem::Type) {
                spec->addArcToPlace(toName, fromName, 1);
            } else {
                spec->addArcFromPlace(fromName, toName, 1);
            }
            m_arcSource = nullptr;
            break;
        }
            

        case Tool::Pan:
            // TODO
            break;

        case Tool::Remove: {
            QGraphicsItem *clicked = itemAt(pos, QTransform());
            if (auto *place = qgraphicsitem_cast<PlaceItem *>(clicked)) {
                removeConnectedArcs(place);
                log(QString("Místo %1 smazáno").arg(place->name()));

                removeItem(place);
                spec->removePlace(place->name().toStdString());
                delete place;

                emit selectionCleared();

            } else if (auto *transition = qgraphicsitem_cast<TransitionItem *>(clicked)) {
                removeConnectedArcs(transition);
                log(QString("Přechod %1 smazán").arg(transition->name()));

                removeItem(transition);
                spec->removeTransition(transition->name().toStdString());
                delete transition;

                emit selectionCleared();

            } else if (auto *arc = qgraphicsitem_cast<ArcItem *>(clicked)) {
                log(QString("Hrana z %1 -> %2 smazána").arg(nameOf(arc->fromItem()), nameOf(arc->toItem())));
                removeItem(arc);
                spec->removeArc(arc);
                delete arc;

                emit selectionCleared();
            }
            break;
        }

        case Tool::Select:
        default:
            if (event->modifiers() & Qt::ControlModifier)
                event->setModifiers(Qt::NoModifier);

            QGraphicsScene::mousePressEvent(event);

            if (auto *place = qgraphicsitem_cast<PlaceItem *>(mouseGrabberItem()))
                emit placeSelected(place);
            else if (selectedItems().isEmpty())
                emit selectionCleared();
            else if (auto *place = qgraphicsitem_cast<PlaceItem *>(selectedItems().first()))
                emit placeSelected(place);
            else if (auto *transition = qgraphicsitem_cast<TransitionItem *>(selectedItems().first()))
                emit transitionSelected(transition);
            else if (auto *arc = qgraphicsitem_cast<ArcItem *>(selectedItems().first()))
                emit arcSelected(arc);
            else
                emit selectionCleared();
            break;
        }
    }
    else if (event->button() == Qt::RightButton){
        QGraphicsItem *clicked = itemAt(event->scenePos(), QTransform());
        if (auto *place = qgraphicsitem_cast<PlaceItem *>(clicked)) {
            showPlaceContextMenu(place, event->screenPos());
        }
        else if (auto *transition = qgraphicsitem_cast<TransitionItem *>(clicked)) {
            showTransitionContextMenu(transition, event->screenPos());
        }
        else if (auto *arc = qgraphicsitem_cast<ArcItem *>(clicked)) {
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
    QAction *removeOne = menu.addAction("Odstraň token");
    QAction *setZero = menu.addAction("Vynulovat tokeny");
    menu.addSeparator();
    QAction *remove = menu.addAction("Smazat místo");

    removeOne->setEnabled(place->tokens() > 0);
    setZero->setEnabled(place->tokens() > 0);

    QAction *chosen = menu.exec(screenPos);
    if (chosen == addOne) {
        place->addToken();
        log(QString("%1: přidán jeden token").arg(place->name()));
    }
    else if (chosen == removeOne) {
        place->removeToken();
        log(QString("%1: odebrán jeden token").arg(place->name()));
    }
    else if (chosen == setZero) {
        place->setTokens(0);
        log(QString("%1: tokeny vynulovány").arg(place->name()));
    }
    else if (chosen == remove) {
        removeConnectedArcs(place);
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
            removeItem(item);
            delete item;
        }
        return;
    }
    QGraphicsScene::keyPressEvent(event);
}

void PetriScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    for (QGraphicsItem *item : items()) {
        if (auto *arc = qgraphicsitem_cast<ArcItem *>(item)) {
            arc->updatePosition();
        } else if (auto *place = qgraphicsitem_cast<PlaceItem *>(item)){
            if (auto *sp = spec->getPlace(place->name().toStdString())) {
                sp->x = place->center().x();
                sp->y = place->center().y();
            }
        } else if (auto *transition = qgraphicsitem_cast<TransitionItem *>(item)){
            if (auto *st = spec->getTransition(transition->name().toStdString())) {
                st->x = transition->center().x();
                st->y = transition->center().y();
            }
        }
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void PetriScene::removeConnectedArcs(QGraphicsItem *node)
{
    const QList<QGraphicsItem *> allItems = items();
    for (QGraphicsItem *item : allItems) {
        auto *arc = qgraphicsitem_cast<ArcItem *>(item);
        if (!arc || (arc->fromItem() != node && arc->toItem() != node))
            continue;
        
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

    log(QString("Hrana %1 → %2 smazána").arg(nameOf(arc->fromItem()), nameOf(arc->toItem())));
    removeItem(arc);
    spec->removeArc(arc);
    delete arc;
}

void PetriScene::drawArc(QGraphicsItem *target)
{
    for (QGraphicsItem *item : items()) {
        if (auto *a = qgraphicsitem_cast<ArcItem *>(item)) {
            
            if (a->fromItem() == m_arcSource && a->toItem() == target){
                cancelArc();
                return;
            }

            if (a->fromItem() == target && a->toItem() == m_arcSource){
                cancelArc();
                return;
            }

        }
    }

    setNodeHighlight(m_arcSource, false);
    ArcItem *arc = new ArcItem(m_arcSource, target);
    addItem(arc);

    log(QString("Hrana přidána: %1 → %2").arg(nameOf(m_arcSource), nameOf(target)));

    if (auto *t = qgraphicsitem_cast<TransitionItem *>(m_arcSource))
        emit transitionSelected(t);
    else if (auto *t = qgraphicsitem_cast<TransitionItem *>(target))
        emit transitionSelected(t);
}

void PetriScene::cancelArc()
{
    setNodeHighlight(m_arcSource, false);
    m_arcSource = nullptr;
}

void PetriScene::applyTheme(const Theme &theme) {
    const QList<QGraphicsItem *> allItems = items();
    for (QGraphicsItem *item : allItems){
        if (auto *p = qgraphicsitem_cast<PlaceItem *>(item))
            p->applyTheme(theme);
        else if (auto *t = qgraphicsitem_cast<TransitionItem *>(item))
            t->applyTheme(theme);
        else if (auto *a = qgraphicsitem_cast<ArcItem *>(item))
            a->applyTheme(theme);
    }

    setBackgroundBrush(theme.sceneBackground);
}

void PetriScene::onDataReceived(picojson::object &data) {
    // TODO: We're not checking the types of values in the JSON here too much
    if(data.find("places") == data.end())
        return;
    object places = value(data["places"]).get<object>();
    for(QGraphicsItem *item : items()) {
        if(PlaceItem *p = qgraphicsitem_cast<PlaceItem*>(item)) {
            const std::string placeName = p->name().toStdString();
            if(places.find(placeName) == places.end())
                continue;
            object currentPlace = places[placeName].get<object>();
            int tokens = currentPlace["currentTokens"].get<int64_t>();
            p->setTokens(tokens);
        }
    }
}