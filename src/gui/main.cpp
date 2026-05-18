/**
 * @file main.cpp
 * @author Dalibor Kalina, xkalin16
 * @brief Entry point for the GUI application
 */

#include "mainwindow.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
