#include "qtinterp.h"

QtInterpreter::QtInterpreter(QObject *parent) : QObject(parent), Interpreter() {}

void QtInterpreter::outputEvent(const std::string output, const std::string value) {
    emit outputReceived(QString::fromStdString(output), QString::fromStdString(value));
}

void QtInterpreter::printState() {
    if (places.empty()) {
        emit statePrinted("  (no places)");
        return;
    }
    QString result = "-- PLACES --\n";
    for (const auto &[id, place] : places) {
        result += QString("  %1  ITOK: %2  CTOK: %3\n")
                      .arg(QString::fromStdString(id))
                      .arg(place->getInitTokens())
                      .arg(place->getTokenCount());
    }
    emit statePrinted(result.trimmed());
}
