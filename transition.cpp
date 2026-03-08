// Place.cpp - Transition class implementation
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>

#include "petri.hpp"
#include <functional>

void Transition::addEntryEdge(TransitionEdge e) {
    enterEdges.push_back(e);
}

void Transition::addExitEdge(TransitionEdge e) {
    exitEdges.push_back(e);
}

void Transition::setAction(std::function<void(TransitionEventParams)> action) {
    transitionEventAction = action;
}

void Transition::setFireCondition(TransitionFireCondition cond) {
    fireCondition = cond;
}