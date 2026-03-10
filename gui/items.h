#ifndef ITEMS_H
#define ITEMS_H

#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QPainter>

class PlaceItem : public QGraphicsEllipseItem {
public:
    static constexpr qreal RADIUS = 30.0;
    explicit PlaceItem(QPointF center, QGraphicsItem *parent = nullptr) :
        QGraphicsEllipseItem(
              center.x() - RADIUS, center.y() - RADIUS,
              RADIUS*2, RADIUS*2, parent) {
        setBrush(Qt::white);
        setPen(QPen(Qt::black, 2));
        setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    }

    QPointF center() const {return mapToScene(rect().center());}
};

class TransitionItem : public QGraphicsEllipseItem {
public:
    static constexpr qreal W = 20.0;
    static constexpr qreal H = 60.0;
    explicit TransitionItem(QPointF center, QGraphicsItem *parent = nullptr) :
        QGraphicsEllipseItem(
            center.x() - W/2, center.y() - H/2,
            W, H, parent) {
        setBrush(Qt::black);
        setPen(QPen(Qt::black, 2));
        setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    }

    QPointF center() const {return mapToScene(rect().center());}
};

//TODO ArcItem

#endif // ITEMS_H
