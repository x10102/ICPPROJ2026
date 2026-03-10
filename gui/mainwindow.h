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

/**
 * @brief Hlavní okno aplikace.
 *
 * Obsahuje toolbar s nástroji editoru a scénu.
 */
class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief Vytvoří toolbar s tlačítky pro výběr nástrojů.
    void setupToolbar();

    /**
     * @brief Přepne aktivní nástroj a vizuálně označí příslušné tlačítko.
     * @param tool   Nástroj k aktivaci
     * @param action Tlačítko toolbaru odpovídající nástroji
     */
    void setActiveTool(Tool tool, QAction *action);

    PetriScene    *m_scene        = nullptr; ///< Scéna Petriho sítě
    QGraphicsView *m_view         = nullptr; ///< Pohled na scénu
    QAction       *m_activeAction = nullptr; ///< Aktuálně aktivní tlačítko toolbaru
};

#endif // MAINWINDOW_H
