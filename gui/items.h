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

class ArcItem : public QGraphicsLineItem {
public:
    ArcItem(QGraphicsItem *from, QGraphicsItem *to, QGraphicsItem *parent = nullptr) :
        QGraphicsLineItem(parent), m_from(from), m_to(to){
        setPen(QPen(Qt::black, 2));
        updatePosition();
    }

    void updatePosition() {
        QPointF a = nodeCenter(m_from);
        QPointF b = nodeCenter(m_to);
        setLine(QLineF(a,b));
    }

    QGraphicsItem *fromItem() const { return m_from;}
    QGraphicsItem *toItem() const { return m_to;}

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        QGraphicsLineItem::paint(painter, option, widget);

        QLineF l = line();
        if (l.length() < 1.0)
            return;

        const qreal arrowSize = 12.0;
        QPointF tip = l.p2();
        double angle = std::atan2(-l.dy(), l.dx());

        QPointF p1 = tip + QPointF(
                        std::sin(angle + M_PI / 3 - M_PI) * arrowSize,
                        std::cos(angle + M_PI / 3 - M_PI) * arrowSize);
        QPointF p2 = tip + QPointF(
                        std::sin(angle - M_PI / 3 + M_PI) * arrowSize,
                        std::cos(angle - M_PI / 3 + M_PI) * arrowSize);

        painter->setBrush(Qt::black);
        painter->drawPolygon(QPolygonF({tip, p1, p2}));
    }

private:
    static QPointF nodeCenter(QGraphicsItem *item){
        if(auto *p = dynamic_cast<PlaceItem *>(item))
            return p->center();
        if(auto *t = dynamic_cast<TransitionItem *>(item))
            return t->center();
        return item->scenePos();
    }

    QGraphicsItem *m_from;
    QGraphicsItem *m_to;
};

#endif // ITEMS_H
