
#include <map>

#include "petri.hpp"
#include "interp.hpp"

void Interpreter::add_transition(Transition t) {
    this->transitions[t.identifier] = t;
}

void Interpreter::add_place(Place p) {
    this->places[p.identifier] = p;
}