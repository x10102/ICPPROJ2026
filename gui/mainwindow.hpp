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
#include "geninterp.hpp"
#include "gui/picojson.h"
#include "petriscene.hpp"
#include "terminaltab.hpp"
#include "theme.hpp"
#include "editorstate.hpp"
#include "udpconnector.hpp"

class QtInterpreter;

/**
 * @brief Main application window
 *
 * Contains the editor scene and toolbars
 *
 * @todo Okomentovat neokomentovane
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

    /// @brief Triggers interpreter codegen and compilation
    void compileInterpreter();

    void applyTheme(const Theme &theme);

    /**
     * @brief Přepne aktivní nástroj a vizuálně označí příslušné tlačítko.
     * @param tool Nástroj k aktivaci
     * @param btn  Tlačítko panelu odpovídající nástroji
     */
    void setActiveTool(Tool tool, QPushButton *btn);

    void populateTransitionSidebar(TransitionItem *transition);
    void clearArcRows();

    PetriScene     *m_scene             = nullptr; ///< Scéna Petriho sítě
    QGraphicsView  *m_view              = nullptr; ///< Pohled na scénu
    QPushButton    *m_activeToolBtn     = nullptr; ///< Aktuálně aktivní tlačítko panelu

    QFrame         *m_toolPanel         = nullptr; ///< Plovoucí panel nástrojů (vlevo nahoře)
    QFrame         *m_simPanel          = nullptr; ///< Plovoucí panel simulace (vpravo dole)
    QPushButton    *m_runBtn            = nullptr;

    bool            m_isPanning         = false;
    QPoint          m_panLastPos;

    QDockWidget    *m_dock              = nullptr;
    QLineEdit      *m_nameEdit          = nullptr;
    QLineEdit      *m_actionEdit        = nullptr;

    QSpinBox       *m_tokenSpin         = nullptr;
    QLabel         *m_tokenLabel        = nullptr;

    QLineEdit      *m_fireCondEdit      = nullptr;
    QLabel         *m_fireCondLabel     = nullptr;

    PlaceItem      *m_editedPlace       = nullptr;
    TransitionItem *m_editedTransition  = nullptr;
    ArcItem        *m_editedArc         = nullptr;

    QWidget        *m_arcPanel          = nullptr;
    QVBoxLayout    *m_arcLayout         = nullptr;
    QWidget        *m_arcWeightPanel    = nullptr;
    QSpinBox       *m_arcWeightSpin     = nullptr;
    QLabel         *m_arcWeightLabel    = nullptr;

    QDockWidget    *m_terminalDock      = nullptr;
    QPlainTextEdit *m_terminal          = nullptr; ///< GUI log tab
    QPlainTextEdit *m_build_terminal    = nullptr; /// Build log tab
    QLineEdit      *m_terminalInput     = nullptr;

    UdpConnector    *m_receiver          = nullptr;
    QThread        *m_receiverThread    = nullptr; 

    PetriNetworkSpec m_spec;
    InterpreterGenerator m_generator;
};



#endif // MAINWINDOW_H
