/**
 * @file items.hpp
 * @author Dalibor Kalina, xkalin16, Adam Šrámek, xsramea00
 * @brief Graphical representation of Petri net elements: places, transitions and arcs.
 */

#ifndef ITEMS_H
#define ITEMS_H

#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QPainter>
#include <cmath>
#include <qchar.h>
#include <qgraphicsitem.h>
#include "theme.hpp"

/**
 * @brief A mixin class used for named items
 * 
 * (This is done to simplify the code, a pattern that was used often was 
 *  checking the type of a graphics item and then q_graphicsitem casting to either
 *  PlaceItem or TransitionItem, both of which have a name getter and setter anyway.
 *  This avoids it.)
 */
class INamed {
    public:
    /// @brief Returns the name
    QString name() const { return m_name;}
    /**
     * @brief Sets the name
     * @param name New name
     */
    void setName(const QString &name) {
        m_name = name;
    }
    protected:
    QString m_name;
};

/**
 * @brief Enumeration of custom graphics item types for type identification
 * Is used in the type() method of each item to allow for q_graphicsitem_cast
 */
enum ItemTypes {
    PlaceItemType = QGraphicsItem::UserType + 1,
    TransitionItemType = QGraphicsItem::UserType + 2,
    ArcItemType = QGraphicsItem::UserType + 3
};

/**
 * @brief Graphical representation of a place in a Petri net.
 *
 * Shows a place as a circle with the number of tokens in the center.
 * The place can be dragged with the mouse.
 * Its token count and other properties can be edited in sidebar.
 */
class PlaceItem : public QGraphicsEllipseItem, public INamed {
public:
    static constexpr qreal RADIUS = 30.0; ///< Radius of the place circle in pixels

    /**
     * @brief Creates a place item centered at the given scene position
     * @param center Center position of the place in scene coordinates
     * @param parent Parent graphics item
     */
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

    /// @brief Returns the center of the place in scene coordinates.
    QPointF center() const {
        return mapToScene(rect().center());
    }

    /// @brief Returns the current number of tokens in the place.
    int tokens() const { 
        return m_tokens;
    }

    /// @brief Adds one token to the place.
    void addToken() {
        m_tokens = qMax(0, m_tokens+1);
        update();
    }
    /// @brief Removes one token from the place (minimum 0).
    void removeToken() {
        m_tokens = qMax(0, m_tokens-1);
        update();
    }
    /**
     * @brief Sets the number of tokens to a specified value.
     * @param val New token count
     */
    void setTokens(int val){
        m_tokens = qMax(0,val);
        update();
    }
    /// @brief Returns the action of the place
    QString action() const {
        return m_action;
    }
    /**
     * @brief Sets the action of the place
     * @param action New action
     */
    void setAction(const QString &action) {
        m_action = action;
        update();
    }

    /**
     * @brief Highlights the place by changing its border color. Used when creating a new arc.
     * @param on If true, the place is highlighted; if false, it returns to normal appearance.
     */
    void setHighlighted(bool on) {
        if (on) {
            setPen(QPen(Theme::current().highlightColor, 3));
        }
        else {
            applyTheme(Theme::current());
        }
    }

    /**
     * @brief Applies the given theme to the place item, updating its colors accordingly.
     * @param theme The theme to apply
     */
    void applyTheme(const Theme &theme){
        setBrush(theme.placeBackground);
        setPen(QPen(theme.placeBorder, 2));
        update();
    }

    /// @brief Qt item type identification for q_graphicsitem_cast<PlaceItem*>() to work
    enum {
        Type = PlaceItemType
    };
    int type() const override {
        return Type;
    }


protected:
    /**
     * @brief Custom paint method to draw the place and the token count text in the center.
     * 
     * Overrides the default paint method of QGraphicsEllipseItem to add text drawing on top of the circle.
     * 
     * @param painter The painter object used for drawing
     * @param option Style options for the item
     * @param widget The widget on which the item is being drawn
     */
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
    int m_tokens = 1;           ///< Current (and initial) number of tokens in the place
    QString m_name;             ///< Place name
    QString m_action;           ///< Place action
};



/**
 * @brief Graphical representation of a transition in a Petri net.
 *
 * Displays the transition as a black narrow ellipse.
 * The transition can be dragged with the mouse.
 * Its properties such as action and firing condition can be edited in the sidebar.
 */
class TransitionItem : public QGraphicsEllipseItem, public INamed {
public:
    static constexpr qreal W = 20.0; ///< Width of the transition in pixels
    static constexpr qreal H = 60.0; ///< Height of the transition in pixels

    /**
     * @brief Creates a transition item centered at the given scene position
     * @param center Center position of the transition in scene coordinates
     * @param parent Parent graphics item
     */
    explicit TransitionItem(QPointF center, QGraphicsItem *parent = nullptr) :
        QGraphicsEllipseItem(
            center.x() - W/2, center.y() - H/2,
            W, H, parent) {
        setBrush(Qt::black);
        setPen(QPen(Qt::black, 2));
        setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);

        applyTheme(Theme::current());
    }

    /// @brief Returns the center of the transition in the scene coordinates
    QPointF center() const {
        return mapToScene(rect().center());
    }

    /// @brief Returns the action of the transition
    QString action() const {
        return m_action;
    }

    /**
     * @brief Sets the action of the transition
     * @param action New action
     */
    void setAction(const QString &action) {
        m_action = action;
        update();
    }

    /// @brief Returns the firing condition of the transition
    QString fireCond() const {
        return m_fireCond;
    }

    /// @brief Returns the delay of the transition
    int delay() const {
        return delayMs;
    }

    /// @brief Sets the delay of the transition
    void setDelay(int delay) {
        delayMs = delay;
    }

    /**
     * @brief Sets the firing condition of the transition
     * @param fireCond New firing condition
     */
    void setFireCond(const QString &fireCond) {
        m_fireCond = fireCond;
        update();
    }

    /// @brief Returns the current input event name
    QString inputEvtName() const {
        return m_inputEvent;
    }

    /**
     * @brief Sets the required input event name
     * @param action Event name
     */
    void setInputEvtName(const QString name) {
        m_inputEvent = name;
    }

    /**
     * @brief Highlights the place by changing its border color. Used when creating a new arc.
     * @param on If true, the place is highlighted; if false, it returns to normal appearance.
     */
    void setHighlighted(bool on) {
        if (on) {
            setPen(QPen(Theme::current().highlightColor, 3));
        }
        else {
            applyTheme(Theme::current());
        }
    }

    /**
     * @brief Applies the given theme to the place item, updating its colors accordingly.
     * @param theme The theme to apply
     */
    void applyTheme(const Theme &theme){
        setBrush(theme.transitionColor);
        setPen(QPen(theme.transitionColor, 2));
        update();
    }

    /// @brief Qt item type identification for q_graphicsitem_cast<TransitionItem*>() to work
    enum {
        Type = TransitionItemType
    };
    int type() const override {
        return Type;
    }

private:
    QString m_name;         ///< Transition name
    QString m_inputEvent;   ///< Name of input event
    QString m_action;       ///< Transition action
    QString m_fireCond;     ///< Transition firing condition
    int delayMs = 0;        ///< Transition fire delay
};



/**
 * @brief Graphical representation of an arc in a Petri net.
 *
 * An arc is displayed as a line with an arrow in the middle pointing from the source node to the target node.
 * The arc's weight is displayed as a number inside the arrow.
 * The arc can be selected with the mouse, and its weight can be edited in the sidebar.
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

    /**
     * @brief Extends the bounding rectangle of the arc to include the area around the arrow in the middle of the arc.
     * 
     * There used to be an issue where the arrow in the middle was cut off in certain angles because the default bounding rectangle of the line only included the thin line itself.
     */
    QRectF boundingRect() const override {
        const qreal extra = 50.0;
        return QGraphicsLineItem::boundingRect().adjusted(-extra, -extra, extra, extra);
    }

    /**
     * @brief Extends the clickable area of the arc to include the arrow.
     * 
     * Withouh this, user would have to click on the thin line of the arc to select it, which is very difficult at certain angles
     */
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

    /**
     * @brief Recalculates arc position on screen based on node coordinates
     */
    void updatePosition() {
        QPointF a = nodeCenter(m_from);
        QPointF b = nodeCenter(m_to);
        setLine(QLineF(a,b));
    }

    /// @brief Returns the source node of the arc
    QGraphicsItem *fromItem() const { 
        return m_from;
    }

    /// @brief Returns the target node of the arc
    QGraphicsItem *toItem() const {
        return m_to;
    }

    /// @brief Returns the weight of the arc
    int weight() {
        return m_weight;
    }

    /**
     * @brief Sets the weight of the arc
     * @param w New weight (minimum 1)
     */
    void setWeight(int w) {
        m_weight = qMax(1, w);
        update();
    }

    /**
     * @brief Applies the given theme to the place item, updating its colors accordingly.
     * @param theme The theme to apply
     */
    void applyTheme(const Theme &theme){
        setPen(QPen(theme.arcLine, 2));
        update();
    }

    /// @brief Qt item type identification for q_graphicsitem_cast<ArcItem*>() to work
    enum {
        Type = ArcItemType
    };
    int type() const override {
        return Type;
    }

protected:
    /**
     * @brief Custom paint method to draw the arc line, the arrow in the middle and the weight inside it.
     * 
     * Overrides the default paint method of QGraphicsLineItem to add custom drawing on top of the line.
     * The arrow is drawn as a filled triangle in the middle of the line, pointing from the source node to the target node.
     * The weight is drawn as a number inside the arrow.
     * 
     * @param painter The painter object used for drawing
     * @param option Style options for the item
     * @param widget The widget on which the item is being drawn
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        QGraphicsLineItem::paint(painter, option, widget);

        QLineF l = line();
        if (l.length() < 1.0) // Prevent drawing the arrow if the line is too short
            return;

        // Calculate the points of the arrow triangle
        const qreal h = 28.0;
        const qreal v = 2.0*0.577*h;
        QPointF mid = (l.p1() + l.p2()) / 2.0;
        QPointF dir = (l.p2() - l.p1()) / l.length();
        QPointF n(-dir.y(), dir.x());
        QPointF base1 = mid - dir*h + n*v/2.0;
        QPointF base2 = mid - dir*h - n*v/2.0;
        
        // Draw the arrow as a filled triangle
        painter->setBrush(Theme::current().arcArrow);
        painter->drawPolygon(QPolygonF({mid, base1, base2}));

        painter->setPen(Theme::current().arcText);
        QFont f = painter->font();
        f.setPointSize(8);
        f.setBold(true);
        painter->setFont(f);

        QPointF centre = mid - dir * (2.0 * h / 3.0);

        // Draw the weight text in the center of the arrow
        painter->drawText( QRectF(centre.x() - 12, centre.y() - 12, 24, 24), Qt::AlignCenter, QString::number(m_weight));
    }

private:
    /**
     * @brief Returns the center screen coordinates of a node
     * @param item The node for which to calculate the center
     */
    static QPointF nodeCenter(QGraphicsItem *item){
        return item->mapToScene(item->boundingRect().center());
    }

    QGraphicsItem *m_from;  ///< Source node (PlaceItem or TransitionItem)
    QGraphicsItem *m_to;    ///< target node (PlaceItem or TransitionItem)
    int m_weight = 1;       ///< Arc weight (initial value 1, minimum value 1)
};

#endif // ITEMS_H
