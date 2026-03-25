/**
 * @file qtinterp.h
 * @author Dalibor Kalina, xkalin16
 * @brief Wrapper nad interpretem
 *
 * Odchytává outputEvent() interpretu a přesměruje ho na Qt signál.
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

    /// @brief Vrátí všechny místa a počet tokenů - aktuálně pro testovací účely
    QString stateString() const;

signals:
    /// @brief Signál, který se vyšle při každém Interperter::outputEvent
    void outputReceived(const QString &name, const QString &value);
};

#endif // QTINTERP_H
