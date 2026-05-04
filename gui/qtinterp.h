/**
 * @file qtinterp.h
 * @author Dalibor Kalina, xkalin16
 * @brief Wrapper nad interpretem
 *
 * Odchytává eventy interpretu a přesměruje je na Qt signál.
 */
#ifndef QTINTERP_H
#define QTINTERP_H

#include <QObject>
#include <QString>
#include "interp.hpp"

class QtInterpreter : public QObject, public Interpreter {
    Q_OBJECT
public:
    explicit QtInterpreter(QObject *parent = nullptr);

    /// @brief Přesměruje Interpreter::outputEvent na outputReceived signál.
    void outputEvent(const std::string output, const std::string value) override;
    void printState() override;
    void doTransitionStep();

signals:
    void outputReceived(const QString &name, const QString &value);
    void statePrinted(const QString &state);
    void stepLogged(const QString &msg);
};

#endif // QTINTERP_H
