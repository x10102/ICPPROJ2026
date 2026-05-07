// transition.cpp - Transition class implementation
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>

#include "petri.hpp"
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>

#include "debug.hpp"
#include "gui/picojson.h"

using std::string;

picojson::object TransitionEdge::json() {
    picojson::object json;
    json["transition"] = picojson::value(this->transition->identifier);
    json["place"] = picojson::value(this->place->identifier);
    json["weight"] = picojson::value(static_cast<double>(this->weight));
    return json;
}

Transition::Transition(std::string identifier) {
    this->identifier = identifier;
    this->fireCondition = {0, ""};
}

picojson::object Transition::json() {
    picojson::object json;
    picojson::array enterEdges;
    picojson::array exitEdges;
    json["name"] = picojson::value(this->identifier);
    for(auto &e : this->enterEdges)
        enterEdges.push_back(picojson::value(e.json()));
    for(auto &e : this->exitEdges)
        enterEdges.push_back(picojson::value(e.json()));
    json["enterEdges"] = picojson::value(enterEdges);
    json["exitEdges"] = picojson::value(exitEdges);
    return json;
}

void Transition::addEntryEdge(TransitionEdge e) {
    enterEdges.push_back(e);
}

void Transition::addEntryEdge(Place *from, uint32_t weight) {
    enterEdges.push_back({from, this, weight});
}

void Transition::addExitEdge(TransitionEdge e) {
    exitEdges.push_back(e);
}

void Transition::addExitEdge(Place *to, uint32_t weight) {
    exitEdges.push_back({to, this, weight});
}

void Transition::setAction(std::function<void(void)> action) {
    transitionEventAction = action;
}

void Transition::setFireCondition(TransitionFireCondition cond) {
    fireCondition = cond;
}

bool Transition::checkGuard(void) {
    // We can do this, boolean comparison is defined on std::function
    // as 'true' if there's a callable target inside
    if(!fireCondition.boolExpr) return true;
    // Evaluate the lambda function and return the result
    return fireCondition.boolExpr();
}

TransitionFireCondition *Transition::getFireCondition() {
    return &fireCondition;
}

// Returns true if a transition has a delayed fire, false otherwise
bool Transition::isDelayed() const {
    return this->fireCondition.delayMs != 0;
}

// Returns true if a transition can fire on a specified input event
bool Transition::firesOnEvent(const std::string s) const {
    if(this->fireCondition.inputEventName.empty())
        return true;
    return this->fireCondition.inputEventName.compare(s) == 0;
}

bool Transition::canFire() {
    if(enterEdges.empty() || exitEdges.empty()) return false;
    for(auto &edge : enterEdges) {
        if(edge.place->getTokenCount() < edge.weight)
            return false;
    }
    return true;
}

void Transition::fire() {
    if(!canFire())
        throw std::logic_error("ERROR: Invalid transition fire");
    LOG_T("Transition fire: %s", identifier.c_str());
    // Consume tokens through entry edges
    for(auto &edge : enterEdges) {
        edge.place->removeTokens(edge.weight);
        LOG_T("Remove %u tokens from %s", edge.weight, edge.place->identifier.c_str());
    }
    // Insert tokens through exit edges
    for(auto &edge : exitEdges) {
        edge.place->addTokens(edge.weight);
        LOG_T("Add %u tokens to %s", edge.weight, edge.place->identifier.c_str());
    }
    // Run the transition action
    if(transitionEventAction) {
        transitionEventAction();
    }
}

void Transition::setIdentifier(const std::string identifier) {
    this->identifier = identifier;
}

const std::vector<TransitionEdge>& Transition::getEntryEdges() const {
    return enterEdges;
}

const std::vector<TransitionEdge>& Transition::getExitEdges() const {
    return exitEdges;
}

void Transition::setEntryEdgeWeight(Place *from, uint32_t weight) {
    for (auto &edge : enterEdges)
        if (edge.place == from) { edge.weight = weight; return; }
}

void Transition::setExitEdgeWeight(Place *to, uint32_t weight) {
    for (auto &edge : exitEdges)
        if (edge.place == to) { edge.weight = weight; return; }
}

bool Transition::removeEntryEdge(Place *from) {
    int found = -1;
    for (int i = 0; i < (int)enterEdges.size(); i++) {
        if (enterEdges[i].place == from) {
            found = i;
            break;
        }
    }
    if (found == -1) {
        return false;
    }
    enterEdges.erase(enterEdges.begin() + found);
    return true;
}

bool Transition::removeExitEdge(Place *to) {
    int found = -1;
    for (int i = 0; i < (int)exitEdges.size(); i++) {
        if (exitEdges[i].place == to) {
            found = i;
            break;
        }
    }
    if (found == -1) {
        return false;
    }
    exitEdges.erase(exitEdges.begin() + found);
    return true;
}