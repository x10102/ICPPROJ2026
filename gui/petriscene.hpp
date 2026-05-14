/**
 * @file petriscene.h
 * @author Dalibor Kalina, xkalin16
 * @brief Scéna editoru Petriho sítě a výčet nástrojů.
 */

#ifndef PETRISCENE_H
#define PETRISCENE_H

#include <QGraphicsScene>
#include "terminaltab.hpp"
#include "items.hpp"
#include "editorstate.hpp"

/**
 * @brief Aktivní nástroj editoru.
 */
enum class Tool {
    Pan,           ///< Pohyb po ploše
    Select,        ///< Výběr jednoho a více uzlů tažením
    AddPlace,      ///< Přidání místa kliknutím do scény
    AddTransition, ///< Přidání přechodu kliknutím do scény
    AddArc,        ///< Kreslení hrany mezi dvěma uzly
    Remove         ///< Smazání uzlu kliknutím
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

    /// @brief Nastaví novou barevnou paletu objektům 
    void applyTheme(const Theme &theme);

    void setNetworkSpec(PetriNetworkSpec *spec);

public slots:
    void onDataReceived(picojson::object &data);

signals:
    /// @brief Signál emitován při výběru místa v režimu Select.
    void placeSelected(PlaceItem *place);
    /// @brief Signál emitován při výběru přechodu v režimu Select.
    void transitionSelected(TransitionItem *transition);
    /// @brief Signál emitován při výběru hrany v režimu Select.
    void arcSelected(ArcItem *arc);
    /// @brief Signál emitován při zrušení výběru - kliknutí do prázdma.
    void selectionCleared();
    /// @brief Signál emitován při logovatelné akci uživatele.
    void logMessage(const QString &message, const TerminalTab tab);
    

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
    /// @brief Zobrazí kontextové menu pro danou hranu.
    void showArcContextMenu(ArcItem *arc, QPoint screenPos);
    /// @brief Kreslí hranu mezi zdrojovým uzlem a cílem.
    void drawArc(QGraphicsItem *target);
    /// @brief Zruší kreslení hrany.
    void cancelArc();
    /// @brief Emituje logMessage se časovým razítkem.
    void log(const QString &msg);
    /// @brief Odstraní všechny arcy napojené na uzel a vyčistí hrany v interpretu.
    void removeConnectedArcs(QGraphicsItem *node);

    Tool m_tool = Tool::Select;           ///< Aktuálně aktivní nástroj
    QGraphicsItem *m_arcSource = nullptr; ///< TODO
    int m_placeCounter         = 0;       ///< Čítač pro generování jmen míst
    int m_transitionCounter    = 0;       ///< Čítač pro generování jmen přechodů

    PetriNetworkSpec *spec;
};

#endif // PETRISCENE_H
