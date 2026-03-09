#include "petri.hpp"
#include "interp.hpp"

void Interpreter::add_transition(Transition t) {
    this->transitions[t.identifier] = t;
}

void Interpreter::add_place(Place p) {
    this->places[p.identifier] = p;
}

void Interpreter::do_immediate_transitions() {
    // Iterating over a hash map eww
    for(auto &transition : transitions) {
        
    }
}