#include "petriscene.hpp"
#include <QGraphicsSceneMouseEvent>
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
            log(QString("Místo %1 přidáno").arg(name));
            break;
        }

        case Tool::AddTransition: {
            auto *transition = new TransitionItem(pos);
            QString name = QString("t%1").arg(++m_transitionCounter);
            transition->setName(name);
            addItem(transition);
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

                    if ((dynamic_cast<PlaceItem *>(m_arcSource) == nullptr) && (dynamic_cast<PlaceItem *>(clicked) == nullptr)){
                        cancelArc();
                    } else if ((dynamic_cast<TransitionItem *>(m_arcSource) == nullptr) && (dynamic_cast<TransitionItem *>(clicked) == nullptr)){
                        cancelArc();
                    }
                    else {
                        drawArc(clicked);
                    }
                }
                else {
                    cancelArc();
                }
            }
            break;
        }

        case Tool::Pan:
            break;

        case Tool::Remove: {
            QGraphicsItem *clicked = itemAt(pos, QTransform());
            if (auto *place = dynamic_cast<PlaceItem *>(clicked)) {
                removeConnectedArcs(place);
                log(QString("Místo %1 smazáno").arg(place->name()));

                removeItem(place);
                delete place;

                emit selectionCleared();

            } else if (auto *transition = dynamic_cast<TransitionItem *>(clicked)) {
                removeConnectedArcs(transition);
                log(QString("Přechod %1 smazán").arg(transition->name()));

                removeItem(transition);
                delete transition;

                emit selectionCleared();

            } else if (auto *arc = dynamic_cast<ArcItem *>(clicked)) {
                showArcContextMenu(arc, event->screenPos());
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
            else if (auto *arc = dynamic_cast<ArcItem *>(selectedItems().first()))
                emit arcSelected(arc);
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

        auto nameOf = [](QGraphicsItem *item) -> QString {
            if (auto *p = dynamic_cast<PlaceItem *>(item)) 
                return p->name();
            if (auto *t = dynamic_cast<TransitionItem *>(item))
                return t->name();
            return "?";
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
        return "?";
    };

    log(QString("Hrana %1 → %2 smazána").arg(nameOf(arc->fromItem()), nameOf(arc->toItem())));
    removeItem(arc);
    delete arc;
}

void PetriScene::drawArc(QGraphicsItem *target)
{
    setNodeHighlight(m_arcSource, false);
    ArcItem *arc = new ArcItem(m_arcSource, target);
    addItem(arc);

    auto nameOf = [](QGraphicsItem *item) -> QString {
        if (auto *p = dynamic_cast<PlaceItem *>(item))
            return p->name();
        if (auto *t = dynamic_cast<TransitionItem *>(item))
            return t->name();
        return "?";
    };
    log(QString("Hrana přidána: %1 → %2").arg(nameOf(m_arcSource), nameOf(target)));

    m_arcSource = nullptr;
}

void PetriScene::cancelArc()
{
    setNodeHighlight(m_arcSource, false);
    m_arcSource = nullptr;
}

void PetriScene::applyTheme(const Theme &theme) {
    const QList<QGraphicsItem *> allItems = items();
    for (QGraphicsItem *item : allItems){
        if (auto *p = dynamic_cast<PlaceItem *>(item))
            p->applyTheme(theme);
        else if (auto *t = dynamic_cast<TransitionItem *>(item))
            t->applyTheme(theme);
        else if (auto *a = dynamic_cast<ArcItem *>(item))
            a->applyTheme(theme);
    }

    setBackgroundBrush(theme.sceneBackground);
}
