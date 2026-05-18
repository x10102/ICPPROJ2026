/**
 * @file petriscene.hpp
 * @author Dalibor Kalina, xkalin16
 * @brief PetriNet Scene editor interface
 */

#ifndef PETRISCENE_H
#define PETRISCENE_H

#include <QGraphicsScene>
#include "terminaltab.hpp"
#include "items.hpp"
#include "editorstate.hpp"

/**
 * @brief Active tool in the editor used to determine the behavior of mouse clicks in the scene
 */
enum class Tool {
    Pan,           ///< Moving the view by dragging
    Select,        ///< Selecting one or more nodes by clicking or dragging
    AddPlace,      ///< Adding a place by clicking into the scene
    AddTransition, ///< Adding a transition by clicking into the scene
    AddArc,        ///< Adding an arc between two nodes
    Remove         ///< Removing a node or an arc by clicking on it
};

/**
 * @brief PetriScene is an extension of QGraphicsScene that implements the logic of petri net.
 * It handles mouse events to add/remove nodes and arcs.
 */
class PetriScene : public QGraphicsScene {
    Q_OBJECT

public:
    static constexpr qreal SCENE_W = 2000.0; ///< Width of the scene in pixels
    static constexpr qreal SCENE_H = 2000.0; ///< Height of the scene in pixels

    explicit PetriScene(QObject *parent = nullptr);

    /**
     * @brief Set active tool in the editor
     * @param tool New active tool
     */
    void setTool(Tool tool);

    /// @brief Returns the currently selected tool.
    Tool currentTool() const {return m_tool;}

    /**
     * @brief Set new color theme to all items in the scene
     * @param theme New color theme
     */
    void applyTheme(const Theme &theme);

    /**
     * @brief Set the PetriNetworkSpec that the scene edits.
     * @param spec Pointer to the PetriNetworkSpec instance
     */
    void setNetworkSpec(PetriNetworkSpec *spec);

public slots:
    void onDataReceived(picojson::object &data);

signals:

    /** @brief Signal emmited when a place is selected in Select mode.
     *  @param place The selected place.
     */
    void placeSelected(PlaceItem *place);

    /** @brief Signal emmited when a transition is selected in Select mode.
     *  @param transition The selected transition.
     */
    void transitionSelected(TransitionItem *transition);

    /** @brief Signal emmited when an arc is selected in Select mode.
     *  @param arc The selected arc.
     */
    void arcSelected(ArcItem *arc);

    /// @brief Signal emmited when the selection is cleared - clicking on empty space
    void selectionCleared();
    
    /** @brief Signal emmited to log a message to the terminal
     *  @param message The message to log.
     *  @param tab The terminal tab where the message should be logged.
     */
    void logMessage(const QString &message, const TerminalTab tab);
    

protected:
    /** @brief Handles mouse clicks in the scene to add/remove/select nodes based on active tool.
     *  @param event The mouse event.
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /** @brief Handles mouse movement to update attached arcs while dragging nodes.
     *  @param event The mouse event.
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    /** @brief Handles the Delete key to remove selected nodes/arcs.
     *  @param event The key event.
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    /**
     * @brief Shows a context menu for the given place.
     * The menu contains options to remove the place and edit its properties.
     * 
     * @param place The place for which to show the context menu
     * @param screenPos The position on the screen where the menu should be shown
     */
    void showPlaceContextMenu(PlaceItem *place, QPoint screenPos);

    /**
     * @brief Shows a context menu for the given transition.
     * The menu contains option to remove the transition.
     * 
     * @param transition The transition for which to show the context menu
     * @param screenPos The position on the screen where the menu should be shown
     */
    void showTransitionContextMenu(TransitionItem *transition, QPoint screenPos);

    /**
     * @brief Shows a context menu for the given arc.
     * The menu contains option to remove the arc.
     * 
     * @param arc The arc for which to show the context menu
     * @param screenPos The position on the screen where the menu should be shown
     */
    void showArcContextMenu(ArcItem *arc, QPoint screenPos);

    /**
     * @brief Draws and arc from currently selected node to target node.
     * @todo If the arc already exists, it could be removed instead of doing nothing.
     * @param target The node to which the arc should be drawn.
     */
    bool drawArc(QGraphicsItem *target);

    /**
     * @brief Cancles the arc drawing mode without drawing an arc.
     * Used when the user clicks on an ivalid target or clicks on the source node again.
     */
    void cancelArc();

    /** @brief Emits logMessage signal with the given message to log it into terminal.
     *  @param msg The message to log.
     */
    void log(const QString &msg);

    /**
     * @brief Removes all arcs connected to the given node. Used when deleting a node to also remove attached arcs.
     * @param node The node for which to remove connected arcs.
     */
    void removeConnectedArcs(QGraphicsItem *node);



    Tool m_tool = Tool::Select;             ///< Currently active tool in the editor
    QGraphicsItem *m_arcSource = nullptr;   ///< Source node for arc drawing in AddArc mode, nullptr if not currently drawing an arc
    int m_placeCounter         = 0;         ///< Counter for generating names of places
    int m_transitionCounter    = 0;         ///< Counter for generating names of transitions

    PetriNetworkSpec *spec; ///< Pointer to the PetriNetworkSpec instance that the scene edits.
};

#endif // PETRISCENE_H
