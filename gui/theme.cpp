#include "theme.hpp"

Theme Theme::current_theme = Theme::light();

Theme Theme::light() {
    Theme t;
    t.placeBackground   = Qt::white;
    t.placeBorder       = Qt::black;
    t.placeText         = Qt::black;
    t.transitionColor   = Qt::black;
    t.arcLine           = Qt::lightGray;
    t.arcArrow          = Qt::lightGray;
    t.arcText           = Qt::black;
    t.highlightColor    = QColor(255, 140, 0);
    t.sceneBackground   = Qt::white;
    t.windowBackground  = Qt::white;
    t.windowText        = Qt::black;
    return t;
}

Theme Theme::dark() {
    Theme t;
    t.placeBackground   = QColor(50, 50, 50);
    t.placeBorder       = QColor(200, 200, 200);
    t.placeText         = QColor(220, 220, 220);
    t.transitionColor   = QColor(180, 180, 180);
    t.arcLine           = QColor(120, 120, 120);
    t.arcArrow          = QColor(120, 120, 120);
    t.arcText           = QColor(220, 220, 220);
    t.highlightColor    = QColor(255, 140, 0);
    t.sceneBackground   = Qt::gray;
    t.windowBackground  = Qt::lightGray;
    t.windowText        = Qt::black;
    return t;
}

const Theme &Theme::current() {
    return current_theme;
}

void Theme::apply(const Theme &theme) {
    current_theme = theme;
}