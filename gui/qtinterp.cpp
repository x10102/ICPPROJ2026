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

    result += "\n-- ARCS --\n";
    bool anyArc = false;
    for (const auto &[id, transition] : transitions) {
        for (const auto &edge : transition->getEntryEdges()) {
            result += QString("  %1 --> %2  (w:%3)\n")
                          .arg(QString::fromStdString(edge.place->identifier))
                          .arg(QString::fromStdString(id))
                          .arg(edge.weight);
            anyArc = true;
        }
        for (const auto &edge : transition->getExitEdges()) {
            result += QString("  %1 --> %2  (w:%3)\n")
                          .arg(QString::fromStdString(id))
                          .arg(QString::fromStdString(edge.place->identifier))
                          .arg(edge.weight);
            anyArc = true;
        }
    }
    if (!anyArc)
        result += " (no arcs)\n";

    emit statePrinted(result.trimmed());
}
