/**
 * @file petriscene.h
 * @author Dalibor Kalina, xkalin16
 * @brief Scéna editoru Petriho sítě a výčet nástrojů.
 */

#ifndef PETRISCENE_H
#define PETRISCENE_H

#include <QGraphicsScene>
#include "items.h"

/**
 * @brief Aktivní nástroj editoru.
 */
enum class Tool {
    Select,        ///< Výběr a přesun uzlů
    AddPlace,      ///< Přidání místa kliknutím do scény
    AddTransition, ///< Přidání přechodu kliknutím do scény
    AddArc         ///< Kreslení hrany mezi dvěma uzly
};

/**
 * @brief Scéna editoru Petriho sítě.
 *
 * Rozšiřuje QGraphicsScene o logiku editoru:
 * přidávání uzlů, kreslení hran a kontextové menu.
 */
class PetriScene : public QGraphicsScene {
    Q_OBJECT

public:
    static constexpr qreal SCENE_W = 2000.0; ///< Šířka scény v pixelech
    static constexpr qreal SCENE_H = 2000.0; ///< Výška scény v pixelech

    explicit PetriScene(QObject *parent = nullptr);

    /**
     * @brief Nastaví aktivní nástroj editoru.
     * @param tool Nový aktivní nástroj
     */
    void setTool(Tool tool);

    /// @brief Vrátí aktuálně vybraný nástroj.
    Tool currentTool() const {return m_tool;}

signals:
    /// @brief Signál emitován při výběru místa v režimu Select.
    void placeSelected(PlaceItem *place);
    /// @brief Signál emitován při výběru přechodu v režimu Select.
    void transitionSelected(TransitionItem *transition);
    /// @brief Signál emitován při zrušení výběru - kliknutí do prázdma.
    void selectionCleared();

protected:
    /// @brief Zpracuje stisknutí tlačítka myši.
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    /// @brief Zpracuje pohyb myši (aktuálně bez využití).
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    /// @brief Zpracuje stisk klávesy.
    void keyPressEvent(QKeyEvent *event) override;

private:
    /// @brief Zobrazí kontextové menu pro dané místo.
    void showPlaceContextMenu(PlaceItem *place, QPoint screenPos);
    /// @brief Zobrazí kontextové menu pro daný přechod.
    void showTransitionContextMenu(TransitionItem *transition, QPoint screenPos);
    /// @brief TODO
    void drawArc(QGraphicsItem *target);
    /// @brief TODO
    void cancelArc();

    Tool m_tool = Tool::Select; ///< Aktuálně aktivní nástroj
    QGraphicsItem *m_arcSource = nullptr; ///< TODO
};

#endif // PETRISCENE_H
