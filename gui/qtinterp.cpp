#include "qtinterp.h"
#include <algorithm>
#include <thread>

QtInterpreter::QtInterpreter(QObject *parent) : QObject(parent), Interpreter() {}

void QtInterpreter::outputEvent(const std::string output, const std::string value) {
    emit outputReceived(QString::fromStdString(output), QString::fromStdString(value));
}

void QtInterpreter::doTransitionStep() {
    std::vector<std::pair<uint32_t, Transition*>> to_fire;
    std::lock_guard<std::mutex> tr_lock(transition_lock);

    for (auto &[id, t] : transitions) {
        if (t->canFire())
            to_fire.push_back({transitionOrder[id], t.get()});
    }

    std::sort(to_fire.begin(), to_fire.end());

    for (auto &[order, t] : to_fire) {
        if (!t->firesOnEvent(last_input) || !t->checkGuard())
            continue;
        if (!t->isDelayed()) {
            QString msg = QString("[step] %1").arg(QString::fromStdString(t->identifier));
            for (auto &e : t->getEntryEdges()) {
                msg += QString("\n  %1 -%2-> %3") 
                           .arg(QString::fromStdString(e.place->identifier))
                           .arg(e.weight)
                           .arg(QString::fromStdString(t->identifier));
            }
            for (auto &e : t->getExitEdges()){
                msg += QString("\n  %1 +%2-> %3")
                           .arg(QString::fromStdString(t->identifier))
                           .arg(e.weight)
                           .arg(QString::fromStdString(e.place->identifier));
            }
            t->fire();
            emit stepLogged(msg);
        } else {
            uint32_t delay = t->getFireCondition()->delayMs;
            timerThreads.emplace_back([this, delay, t]() {delayedFire(t, delay); });
        }
    }
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
