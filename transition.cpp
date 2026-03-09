// transition.cpp - Transition class implementation
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>

#include "petri.hpp"
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>

#include "debug.hpp"

Transition::Transition(std::string identifier) {
    this->identifier = identifier;
    this->fireCondition = {0, ""};
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

void Transition::setAction(std::function<void(TransitionEventParams)> action) {
    transitionEventAction = action;
}

void Transition::setFireCondition(TransitionFireCondition cond) {
    fireCondition = cond;
}

// Returns true if a transition has a delayed fire, false otherwise
bool Transition::isDelayed() {
    return this->fireCondition.delayMs != 0;
}

// Returns true if a transition can fire on a specified input event
bool Transition::firesOnEvent(std::string s) {
    if(this->fireCondition.inputEventName.empty())
        return true;
    return this->fireCondition.inputEventName.compare(s) == 0;
}

bool Transition::canFire() {
    // TODO: All the other bullshit conditions
    // TODO: Something completely different for the timed transition, probably will need a thread with a polling loop
    for(auto &edge : enterEdges) {
        if(edge.place->getTokenCount() < edge.weight)
            return false;
    }
    return true;
}

void Transition::fire() {
    // TODO: Possibly remove this?
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
}