/**
 * @file mainwindow.h
 * @author Dalibor Kalina, xkalin16
 * @brief Hlavní okno aplikace editoru Petriho sítí.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "petriscene.h"

class QGraphicsView;
class QAction;
class QDockWidget;
class QLineEdit;
class QSpinBox;
class QLabel;
class QVBoxLayout;

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

private:
    /// @brief Vytvoří toolbar s tlačítky pro výběr nástrojů.
    void setupToolbar();
    /// @brief Vytvoří sidebar s popisem uzlů
    void setupSidebar();

    /**
     * @brief Přepne aktivní nástroj a vizuálně označí příslušné tlačítko.
     * @param tool   Nástroj k aktivaci
     * @param action Tlačítko toolbaru odpovídající nástroji
     */
    void setActiveTool(Tool tool, QAction *action);

    void populateTransitionSidebar(TransitionItem *transition);
    void clearArcRows();

    PetriScene     *m_scene             = nullptr; ///< Scéna Petriho sítě
    QGraphicsView  *m_view              = nullptr; ///< Pohled na scénu
    QAction        *m_activeAction      = nullptr; ///< Aktuálně aktivní tlačítko toolbaru

    QDockWidget    *m_dock              = nullptr;
    QLineEdit      *m_nameEdit          = nullptr;

    QSpinBox       *m_tokenSpin         = nullptr;
    QLabel         *m_tokenLabel        = nullptr;

    PlaceItem      *m_editedPlace       = nullptr;
    TransitionItem *m_editedTransition  = nullptr;
    QWidget        *m_arcPanel          = nullptr;
    QVBoxLayout    *m_arcLayout         = nullptr;
};

#endif // MAINWINDOW_H
