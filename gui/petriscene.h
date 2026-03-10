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
    explicit PetriScene(QObject *parent = nullptr);

    /**
     * @brief Nastaví aktivní nástroj editoru.
     * @param tool Nový aktivní nástroj
     */
    void setTool(Tool tool);

    /// @brief Vrátí aktuálně vybraný nástroj.
    Tool currentTool() const {return m_tool;}

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

    Tool m_tool = Tool::Select; ///< Aktuálně aktivní nástroj
};

#endif // PETRISCENE_H
