/**
 * @file mainwindow.hpp
 * @author Dalibor Kalina, xkalin16, Ondřej Turek, xtureko00
 * @brief Main application window of the Petri network editor
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QAction>
#include <QDockWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPoint>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QFrame>
#include <qglobal.h>
#include <qobject.h>
#include <qplaintextedit.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include "geninterp.hpp"
#include "gui/picojson.h"
#include "petriscene.hpp"
#include "terminaltab.hpp"
#include "theme.hpp"
#include "editorstate.hpp"
#include "udpconnector.hpp"
#include "variableeditor.hpp"

class QtInterpreter;

/**
 * @brief Main application window
 *
 * Contains the editor scene and toolbars
 */
class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onDataReceived(picojson::object &data);

private:
    /// @brief Creates a toolbar with tool-picker buttons
    void setupToolbar();
    /// @brief Creates a sidebar with node attributes
    void setupSidebar();
    /// @brief Creates a terminal in the lower part of the window
    void setupTerminal();
    /// @brief Appends a message to the GUI terminal log
    void appendLog(const QString &msg, const TerminalTab tab = GUI);
    /// @brief Creates floating tool and simulation panels over the scene
    void setupFloatingPanels();
    /// @brief Recalculates the simulation panel position to the lower right corner
    void repositionSimPanel();

    /// @brief Lets the user choose a file using the system file picker dialog and saves the net
    bool saveNet();

    /// @brief Opens a file picker to choose a net specification file and loads it
    bool loadNet();

    /// @brief Lets the user choose a directory with the interpreter source
    bool setSourceDir();

    /// @brief A message handler to append Qt logger messages to the terminal
    void terminalMessageHandler(QtMsgType msgType, QMessageLogContext &ctx, const QString &message);

    /// @brief Sets up the UDP receiver thread for communication with the client
    void setupUDPThread();

    /// @brief Sets up paths for the interpreter generator
    void setupSourceGenerator();

    /// @brief Opens a dialog used to change the net properties (name etc.)
    void openNetPropsDialog();

    /// @brief Opens a dialog used to define variables
    void openVariablesDialog();

    /// @brief Triggers interpreter codegen and compilation
    void compileInterpreter();

    /**
     * @brief Applies the given theme to all GUI elements and items in the scene.
     * @param theme The theme to apply
     */
    void applyTheme(const Theme &theme);

    /**
     * @brief Sets the active tool in the editor and updates the corresponding button's checked state.
     * @param tool Tool to activate
     * @param btn Button in the toolbar corresponding to the tool
     */
    void setActiveTool(Tool tool, QPushButton *btn);

    /**
     * @brief Populates the sidebar with editable attributes of the given transition
     * @param transition The transition whose attributes to show aka the transition that has been selected
     */
    void populateTransitionSidebar(TransitionItem *transition);

    bool           m_interpRunnable     = false;    ///< True if interpreter was built and can be run
    bool           m_interpRunning      = false;    ///< True if interpreter is currently running

    PetriScene     *m_scene             = nullptr;  ///< Petri net scene
    QGraphicsView  *m_view              = nullptr;  ///< View for the scene
    QPushButton    *m_activeToolBtn     = nullptr;  ///< Currently active tool button

    QFrame         *m_toolPanel         = nullptr;  ///< Floating tool panel (top left)
    QFrame         *m_simPanel          = nullptr;  ///< Floating simulation panel (bottom right)
    QPushButton    *m_runBtn            = nullptr;  ///< Button to start the simulation in the simulation panel
    QPushButton    *m_stepBtn           = nullptr;  ///< Button to step in the simulation
    QPushButton    *m_contBtn           = nullptr;  ///< Button to fire all transitions in the simulation

    bool            m_isPanning         = false;    ///< Whether the user is currently panning the view by dragging
    QPoint          m_panLastPos;                   ///< Last mouse position during panning, used to calculate the panning offset  

    QDockWidget    *m_dock              = nullptr;  ///< Sidebar dock widget (right)
    QLineEdit      *m_nameEdit          = nullptr;  ///< Line edit for the name of the selected node
    QLineEdit      *m_actionEdit        = nullptr;  ///< Line edit for the action of the selected node

    QSpinBox       *m_tokenSpin         = nullptr;  ///< Spin box for the token count of the selected place
    QLabel         *m_tokenLabel        = nullptr;  ///< Label for the token count spin box in the sidebar

    QLineEdit      *m_fireCondEdit      = nullptr;  ///< Line edit for the fire condition of the selected transition
    QLabel         *m_fireCondLabel     = nullptr;  ///< Label for the fire condition edit in the sidebar

    QLineEdit      *m_evtNameEdit      = nullptr;  ///< Line edit for the input event name of the selected transition
    QLabel         *m_evtNameLabel     = nullptr;  ///< Label for the input event edit in the sidebar

    PlaceItem      *m_editedPlace       = nullptr;  ///< Currently selected place for editing, nullptr if no place is selected
    TransitionItem *m_editedTransition  = nullptr;  ///< Currently selected transition for editing, nullptr if no transition is selected
    ArcItem        *m_editedArc         = nullptr;  ///< Currently selected arc for editing, nullptr if no arc is selected

    QWidget        *m_arcPanel          = nullptr;  ///< Panel in the sidebar for showing arc attributes (only visible when an arc is selected)
    QVBoxLayout    *m_arcLayout         = nullptr;  ///< Layout inside the arc panel to dynamically add attributes of the selected arc
    QWidget        *m_arcWeightPanel    = nullptr;  ///< Panel in the sidebar for showing arc weight attribute
    QSpinBox       *m_arcWeightSpin     = nullptr;  ///< Spin box for the weight of the selected arc
    QLabel         *m_arcWeightLabel    = nullptr;  ///< Label for the arc weight spin box in the sidebar

    QDockWidget    *m_terminalDock      = nullptr;  ///< Terminal dock widget (bottom)
    QTabWidget     *m_terminalTabs      = nullptr;  ///< Tab widget containing the terminal tabs
    QPlainTextEdit *m_terminal          = nullptr;  ///< GUI log tab
    QPlainTextEdit *m_buildTerminal     = nullptr;  ///< Build log tab
    QPlainTextEdit *m_interpTerminal    = nullptr;  ///< Interpreter output tab
    QLineEdit      *m_terminalInput     = nullptr;  ///< Input line for the terminal tab

    VariableMap     variables;

    UdpConnector    *m_receiver         = nullptr;
    QThread         *m_receiverThread   = nullptr; 
    InterpreterGenerator *m_generator   = nullptr;

    PetriNetworkSpec m_spec; ///< The Petri net specification currently being edited, used for code generation and simulation
    
};



#endif // MAINWINDOW_H
