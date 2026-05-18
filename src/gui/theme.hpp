/**
 * @file theme.cpp
 * @author Ondřej Turek, xtureko00, Dalibor Kalina, xkalin16
 * @brief GUI Themes interface
 */

#ifndef THEME_HPP
#define THEME_HPP
#include <QColor>

struct Theme {
    QColor placeBackground;
    QColor placeBorder;
    QColor placeText;

    QColor transitionColor;

    QColor arcLine;
    QColor arcArrow;
    QColor arcText;

    QColor highlightColor;

    QColor sceneBackground;
    QColor windowBackground;
    QColor windowText;

    static Theme light();
    static Theme dark();

    static const Theme &current();
    static void apply(const Theme &theme);


    private:
        static Theme current_theme;
};


#endif