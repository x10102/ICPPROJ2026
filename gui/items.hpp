/**
 * @file items.h
 * @author Dalibor Kalina, xkalin16
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

class Place;      // forward declaration — PlaceItem holds a pointer to its interpreter counterpart
class Transition; // forward declaration — TransitionItem holds a pointer to its interpreter counterpart

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

    void setHighlighted(bool on) {
        if (on) {
            setPen(QPen(QColor(255, 140, 0), 3));
        }
        else {
            setPen(QPen(QColor(0,0,0), 2));
        }
    }


protected:
    /// @brief Vykreslí místo a počet tokenů uprostřed.
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        QGraphicsEllipseItem::paint(painter, option, widget);

        QFont f = painter->font();
        f.setBold(true);
        f.setPointSize(11);
        painter->setFont(f);
        painter->setPen(Qt::black);
        painter->drawText(rect().toRect(), Qt::AlignCenter, QString::number(m_tokens));
    }

private:
    int m_tokens = 1;             ///< Aktuální (a počáteční) počet tokenů
    QString m_name;               ///< Název místa
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

    void setHighlighted(bool on) {
        if (on) {
            setPen(QPen(QColor(255, 140, 0), 3));
        }
        else {
            setPen(QPen(QColor(0,0,0), 2));
        }
    }

private:
    QString m_name;    ///< Název přechodu
};

/**
 * @brief Grafická reprezentace hrany (arc) v Petriho síti.
 *
 * Vykresluje orientovanou hranu se šipkou mezi dvěma uzly.
 *
 * @todo Šipka na konci hrany je vykreslena uprostřed uzlu a nejde vidět
 */
class ArcItem : public QGraphicsLineItem {
public:
    /**
     * @brief Vytvoří hranu mezi dvěma uzly.
     * @param from Zdrojový uzel (PlaceItem nebo TransitionItem)
     * @param to   Cílový uzel (PlaceItem nebo TransitionItem)
     * @param parent Rodičovský grafický prvek
     */
    ArcItem(QGraphicsItem *from, QGraphicsItem *to, QGraphicsItem *parent = nullptr) :
        QGraphicsLineItem(parent), m_from(from), m_to(to){
        setPen(QPen(Qt::lightGray, 2));
        setZValue(-1);
        updatePosition();
    }

    /// @brief Přepočítá pozici hrany podle aktuálních středů uzlů.
    void updatePosition() {
        QPointF a = nodeCenter(m_from);
        QPointF b = nodeCenter(m_to);
        setLine(QLineF(a,b));
    }

    /// @brief Vrátí zdrojový uzel hrany.
    QGraphicsItem *fromItem() const { return m_from;}
    /// @brief Vrátí cílový uzel hrany.
    QGraphicsItem *toItem() const { return m_to;}
    /// @brief TODO
    int weight() {return m_weight;}
    /// @brief TODO
    void setWeight(int w) {m_weight = qMax(0, w);}

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

        painter->setBrush(Qt::lightGray);
        painter->drawPolygon(QPolygonF({tip, p1, p2}));
    }

private:
    /// @brief Vrátí střed uzlu.
    static QPointF nodeCenter(QGraphicsItem *item){
        if(auto *p = dynamic_cast<PlaceItem *>(item))
            return p->center();
        if(auto *t = dynamic_cast<TransitionItem *>(item))
            return t->center();
        return item->scenePos();
    }

    QGraphicsItem *m_from; ///< Zdrojový uzel
    QGraphicsItem *m_to;   ///< Cílový uzel
    int m_weight = 1; ///< TODO
};

#endif // ITEMS_H
