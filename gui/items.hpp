/**
 * @file items.h
 * @author Dalibor Kalina, xkalin16, Adam Šrámek, xsramea00
 * @brief Grafické prvky Petriho sítě pro QGraphicsScene.
 *
 * Obsahuje třídy PlaceItem, TransitionItem a ArcItem
 */

#ifndef ITEMS_H
#define ITEMS_H

#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QPainter>
#include <cmath>
#include "theme.hpp"

class Place;      // forward declaration — PlaceItem holds a pointer to its interpreter counterpart
class Transition; // forward declaration — TransitionItem holds a pointer to its interpreter counterpart

enum ItemTypes {
    PlaceItemType = QGraphicsItem::UserType + 1,
    TransitionItemType = QGraphicsItem::UserType + 2,
    ArcItemType = QGraphicsItem::UserType + 3
};

/**
 * @brief Grafická reprezentace místa (place) v Petriho síti.
 *
 * Zobrazuje místo jako kruh s počtem tokenů uprostřed.
 * Místo lze přetahovat myší a upravovat stav jeho tokenů.
 */
class PlaceItem : public QGraphicsEllipseItem {
public:
    static constexpr qreal RADIUS = 30.0; ///< Poloměr kruhu místa v pixelech

    explicit PlaceItem(QPointF center, QGraphicsItem *parent = nullptr) :
        QGraphicsEllipseItem(
              center.x() - RADIUS, center.y() - RADIUS,
              RADIUS*2, RADIUS*2, parent) {
        setBrush(Qt::white);
        setPen(QPen(Qt::black, 2));
        setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
        setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);

        applyTheme(Theme::current());
    }

    /// @brief Vrátí střed místa v souřadnicích scény.
    QPointF center() const {return mapToScene(rect().center());}

    /// @brief Vrátí aktuální počet tokenů v místě.
    int tokens() const { return m_tokens;}
    /// @brief Přidá jeden token do místa.
    void addToken() {
        m_tokens = qMax(0, m_tokens+1);
        update();
    }
    /// @brief Odebere jeden token z místa (minimum 0).
    void removeToken() {
        m_tokens = qMax(0, m_tokens-1);
        update();
    }
    /// @brief Nastaví počet tokenů na zadanou hodnotu.
    void setTokens(int val){
        m_tokens = qMax(0,val);
        update();
    }
    /// @brief Vrátí jméno místa
    QString name() const { return m_name;}
    /**
     * @brief Nastaví jméno místa
     * @param name Nové jméno
     */
    void setName(const QString &name) {
        m_name = name;
        update();
    }
    /// @brief Vráti akci místa
    QString action() const { return m_action; }
    /**
     * @brief Nastaví akci místa
     * @param action Nová akce
     */
    void setAction(const QString &action) {
        m_action = action;
        update();
    }

    void setHighlighted(bool on) {
        if (on) {
            setPen(QPen(Theme::current().highlightColor, 3));
        }
        else {
            applyTheme(Theme::current());
        }
    }

    void applyTheme(const Theme &theme){
        setBrush(theme.placeBackground);
        setPen(QPen(theme.placeBorder, 2));
        update();
    }

    enum {
        Type = PlaceItemType
    };
    int type() const override {
        return Type;
    }


protected:
    /// @brief Vykreslí místo a počet tokenů uprostřed.
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        QGraphicsEllipseItem::paint(painter, option, widget);

        QFont f = painter->font();
        f.setBold(true);
        f.setPointSize(11);
        painter->setFont(f);
        painter->setPen(Theme::current().placeText);
        painter->drawText(rect().toRect(), Qt::AlignCenter, QString::number(m_tokens));
    }

private:
    int m_tokens = 1;             ///< Aktuální (a počáteční) počet tokenů
    QString m_name;               ///< Název místa
    QString m_action;
};

/**
 * @brief Grafická reprezentace přechodu (transition) v Petriho síti.
 *
 * Zobrazuje přechod jako černý úzký obdélník.
 * Přechod lze přetahovat myší.
 */
class TransitionItem : public QGraphicsEllipseItem {
public:
    static constexpr qreal W = 20.0; ///< Šířka přechodu v pixelech
    static constexpr qreal H = 60.0; ///< Výška přechodu v pixelech
    explicit TransitionItem(QPointF center, QGraphicsItem *parent = nullptr) :
        QGraphicsEllipseItem(
            center.x() - W/2, center.y() - H/2,
            W, H, parent) {
        setBrush(Qt::black);
        setPen(QPen(Qt::black, 2));
        setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);

        applyTheme(Theme::current());
    }

    /// @brief Vrátí střed přechodu v souřadnicích scény.
    QPointF center() const {return mapToScene(rect().center());}
    /// @brief Vrátí jméno přechodu
    QString name() const {return m_name;}
    /**
     * @brief Nastaví jméno přechodu
     * @param name Nové jméno
     */
    void setName(const QString &name) {
        m_name = name;
        update();
    }
    /// @brief Vráti akci místa
    QString action() const { return m_action; }
    /**
     * @brief Nastaví akci přechodu
     * @param action Nová akce
     */
    void setAction(const QString &action) {
        m_action = action;
        update();
    }
    /// @brief Vrátí podmínku odpálení
    QString fireCond() const {return m_fireCond;}
    /**
     * @brief Nastaví podmínku odpálení přechodu
     * @param fireCond Nová podmínka odpálení
     */
    void setFireCond(const QString &fireCond) {
        m_fireCond = fireCond;
        update();
    }
    /// @brief Returns the current input event name
    QString inputEvtName() const {return m_inputEvent;};
    /**
     * @brief Sets the required input event name
     * @param action Event name
     */
    void setInputEvtName(const QString name) {
        m_inputEvent = name;
    }
    void setHighlighted(bool on) {
        if (on) {
            setPen(QPen(Theme::current().highlightColor, 3));
        }
        else {
            applyTheme(Theme::current());
        }
    }

    void applyTheme(const Theme &theme){
        setBrush(theme.transitionColor);
        setPen(QPen(theme.transitionColor, 2));
        update();
    }

    enum {
        Type = TransitionItemType
    };
    int type() const override {
        return Type;
    }

private:
    QString m_name;   ///< Název přechodu
    QString m_inputEvent;
    QString m_action;
    QString m_fireCond;
};

/**
 * @brief Grafická reprezentace hrany (arc) v Petriho síti.
 *
 * Vykresluje orientovanou hranu se šipkou mezi dvěma uzly.
 *
 */
class ArcItem : public QGraphicsLineItem {
public:
    /**
     * @brief Creates an arc between two nodes
     * @param from Source node (PlaceItem or TransitionItem)
     * @param to   Target node (PlaceItem or TransitionItem)
     * @param parent Parent graphics element
     */
    ArcItem(QGraphicsItem *from, QGraphicsItem *to, QGraphicsItem *parent = nullptr) :
        QGraphicsLineItem(parent), m_from(from), m_to(to){
        setPen(QPen(Qt::lightGray, 2));
        setZValue(-1);
        setFlag(ItemIsSelectable);
        updatePosition();

        applyTheme(Theme::current());
    }

    /// @brief Rozšiřuje bounding rectangle, aby Qt neořezával šipku na hraně
    /// Ořezávání se dělo v případech, kdy hrana měla příliš vodorovný nebo vertikální sklon
    QRectF boundingRect() const override {
        const qreal extra = 50.0;
        return QGraphicsLineItem::boundingRect().adjusted(-extra, -extra, extra, extra);
    }

    /// @brief Rozšiřuje clickable tvar hrany o šipku na hraně
    /// Bez tohoto by se dalo kliknout jen na samotnou ~2px širokou čáru
    QPainterPath shape() const override {
        QPainterPath path = QGraphicsLineItem::shape();

        QLineF l = line();
        if (l.length() < 1.0)
            return path;

        const qreal h = 28.0;
        const qreal v = 2.0*0.577*h;

        QPointF mid = (l.p1() + l.p2()) / 2.0;
        QPointF dir = (l.p2() - l.p1()) / l.length();
        QPointF n(-dir.y(), dir.x());

        QPointF base1 = mid - dir*h + n*v/2.0;
        QPointF base2 = mid - dir*h - n*v/2.0;

        path.addPolygon(QPolygonF({mid, base1, base2}));
        return path;
    }

    /// @brief Recalculates arc position on screen based on node coordinates
    void updatePosition() {
        QPointF a = nodeCenter(m_from);
        QPointF b = nodeCenter(m_to);
        setLine(QLineF(a,b));
    }

    /// @brief Returns the source node of the arc
    QGraphicsItem *fromItem() const { return m_from;}
    /// @brief Returns the target node of the arc
    QGraphicsItem *toItem() const { return m_to;}
    /// @brief Returns the weight of the arc
    int weight() {return m_weight;}
    /// @brief TODO
    void setWeight(int w) {
        m_weight = qMax(1, w);
        update();
    }

    void applyTheme(const Theme &theme){
        setPen(QPen(theme.arcLine, 2));
        update();
    }

    enum {
        Type = ArcItemType
    };
    int type() const override {
        return Type;
    }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        QGraphicsLineItem::paint(painter, option, widget);

        QLineF l = line();
        if (l.length() < 1.0)
            return;

        const qreal h = 28.0;
        const qreal v = 2.0*0.577*h;

        QPointF mid = (l.p1() + l.p2()) / 2.0;
        QPointF dir = (l.p2() - l.p1()) / l.length();
        QPointF n(-dir.y(), dir.x());

        QPointF base1 = mid - dir*h + n*v/2.0;
        QPointF base2 = mid - dir*h - n*v/2.0;
        
        painter->setBrush(Theme::current().arcArrow);
        painter->drawPolygon(QPolygonF({mid, base1, base2}));

        painter->setPen(Theme::current().arcText);
        QFont f = painter->font();
        f.setPointSize(8);
        f.setBold(true);
        painter->setFont(f);

        QPointF centre = mid - dir * (2.0 * h / 3.0);

        painter->drawText( QRectF(centre.x() - 12, centre.y() - 12, 24, 24), Qt::AlignCenter, QString::number(m_weight));
    }

private:
    /// @brief Returns the center screen coordinates of a node
    static QPointF nodeCenter(QGraphicsItem *item){
        if(auto *p = qgraphicsitem_cast<PlaceItem *>(item))
            return p->center();
        if(auto *t = qgraphicsitem_cast<TransitionItem *>(item))
            return t->center();
        return item->scenePos();
    }

    QGraphicsItem *m_from; ///< Zdrojový uzel
    QGraphicsItem *m_to;   ///< Cílový uzel
    int m_weight = 1; ///< Váha hrany
};

#endif // ITEMS_H
