/**
 * @file mainwindow.h
 * @author Dalibor Kalina, xkalin16
 * @brief Hlavní okno aplikace editoru Petriho sítí.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include "petriscene.h"

class QGraphicsView;
class QAction;
class QDockWidget;
class QLineEdit;
class QSpinBox;
class QLabel;
class QVBoxLayout;
class QPlainTextEdit;
class QPushButton;
class QFrame;
class QtInterpreter;

/**
 * @brief Hlavní okno aplikace.
 *
 * Obsahuje toolbar s nástroji editoru a scénu.
 *
 * @todo Okomentovat neokomentovane
 */
class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    /// @brief Vytvoří toolbar s tlačítky pro výběr nástrojů.
    void setupToolbar();
    /// @brief Vytvoří sidebar s popisem uzlů
    void setupSidebar();
    /// @brief Vytvoří terminál v dolní části okna
    void setupTerminal();
    /// @brief Přidá zprávu do záložky GUI logu
    void appendLog(const QString &msg);
    /// @brief Přidá zprávu do záložky interpreter logu
    void appendInterpLog(const QString &msg);
    /// @brief Spustí interpreter jako subprocess
    void startInterpreter();
    /// @brief Odešle řádek na stdin interpreteru
    void sendToInterpreter(const QString &text);
    /// @brief Vytvoří plovoucí panely nástrojů a simulace nad scénou.
    void setupFloatingPanels();
    /// @brief Přepočítá pozici panelu simulace do pravého dolního rohu.
    void repositionSimPanel();

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

    QSpinBox       *m_tokenSpin         = nullptr;
    QLabel         *m_tokenLabel        = nullptr;

    PlaceItem      *m_editedPlace       = nullptr;
    TransitionItem *m_editedTransition  = nullptr;
    QWidget        *m_arcPanel          = nullptr;
    QVBoxLayout    *m_arcLayout         = nullptr;

    QDockWidget    *m_terminalDock      = nullptr;
    QPlainTextEdit *m_terminal          = nullptr; ///< GUI log tab
    QPlainTextEdit *m_interpLog         = nullptr; ///< Interpreter output tab
    QLineEdit      *m_terminalInput     = nullptr;

    QtInterpreter  *m_interp            = nullptr;
};

#endif // MAINWINDOW_H
