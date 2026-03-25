#include "qtinterp.h"

QtInterpreter::QtInterpreter(QObject *parent) : QObject(parent), Interpreter() {}

void QtInterpreter::outputEvent(const std::string output, const std::string value) {
    emit outputReceived(QString::fromStdString(output), QString::fromStdString(value));
}

void QtInterpreter::printState() {
    QString result = "-- PLACES --\n";
    if (places.empty()) {
        result += " (no places)\n";
    }
    else {
        for (const auto &[id, place] : places) {
            result += QString("  %1  ITOK: %2  CTOK: %3\n")
                        .arg(QString::fromStdString(id))
                        .arg(place->getInitTokens())
                        .arg(place->getTokenCount());
        }
    }
    result += "\n-- TRANSITIONS --\n";
    if (transitions.empty()) {
        result += " (no transitions)\n";
    }
    else {
        for (const auto &[id, transition] : transitions) {
            result += QString("  %1\n")
                        .arg(QString::fromStdString(id));
        }
    }
    emit statePrinted(result.trimmed());
}
