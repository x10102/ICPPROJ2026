#include "qtinterp.h"

QtInterpreter::QtInterpreter(QObject *parent) : QObject(parent), Interpreter() {}

void QtInterpreter::outputEvent(const std::string output, const std::string value) {
    emit outputReceived(QString::fromStdString(output), QString::fromStdString(value));
}

QString QtInterpreter::stateString() const {
    if (places.empty())
        return "  (no places)";
    QString result;
    for (const auto &[id, place] : places) {
        result += QString("  %1  tokens: %2  (initial: %3)\n")
                      .arg(QString::fromStdString(id))
                      .arg(place->getTokenCount())
                      .arg(place->getInitTokens());
    }
    return result.trimmed();
}
