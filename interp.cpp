#include "petri.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>
#include "interp.hpp"

Interpreter::Interpreter() {
    this->max_order = 0;
}

// Adds a transition to the interpreter
// !IMPORTANT: Transitions added first have priority in conflicts
void Interpreter::addTransition(Transition t) {
    this->transitions[t.identifier] = t;
    // Every transition gets assigned a sequential, unique priority value
    // Lower value => higher priority - transitions added earlier fire first in case of conflict
    this->transitionOrder[t.identifier] = this->max_order++;
}

void Interpreter::addPlace(Place p) {
    this->places[p.identifier] = p;
}

void Interpreter::doImmediateTransitions() {
    // We will store transitions which can be fired here, along with their order
    // After that we sort the array and fire them in order, skipping those that no longer meet the conditions
    std::vector<std::pair<uint32_t, Transition*>> to_fire;

    // If any transitions fired during the last loop, we have to run another check
    uint32_t fire_count;

    do {
        // Clear the list of transitions to fire and reset the count
        to_fire.clear();
        fire_count = 0;

        // Iterating over a hash map eww
        for(auto &transition : transitions) {
        if(transition.second.canFire()) {
            Transition *t = &transition.second;
            uint32_t order = transitionOrder[t->identifier];
            to_fire.push_back({order, t});
        }  
        }
        
        // Pair comparison is defined as comparing the first item, then the second.
        // Comparing Transition pointers does not produce meaningful results but
        // the order numbers are unique, so the pointers will never actually be compared
        std::sort(to_fire.begin(), to_fire.end());

        // Evil C++17 pair unpacking
        for(auto &[order, transition] : to_fire) {
            if(transition->canFire() && !transition->isDelayed()) {
                transition->fire();
                fire_count++;
            }
        }
    } while(fire_count > 0);
}