// interp.cpp - interpreter implementation
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>

#include "petri.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <initializer_list>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "interp.hpp"
#include "debug.hpp"

Interpreter::Interpreter() {
    this->max_order = 0;
}

// Triggers an input event -
// - Sets the appropriate input to the value passed
// - Updates the last input name
// - Triggers transition processing
void Interpreter::inputEvent(const std::string input, const std::string value) {
    LOG_D("Input event: %s=%s", input.c_str(), value.c_str());
    this->inputValues[input] = value;
    this->last_input = input;
    this->doTransitions();
}

bool Interpreter::inputDefined(const std::string input) {
    return this->inputValues.count(input) != 0;
}

// Adds a transition to the interpreter
// !IMPORTANT: Transitions added first have priority in conflicts
void Interpreter::addTransition(const Transition t) {
    this->transitions[t.identifier] = t;
    // Every transition gets assigned a sequential, unique priority value
    // Lower value => higher priority - transitions added earlier fire first in case of conflict
    this->transitionOrder[t.identifier] = this->max_order++;
}

void Interpreter::addPlace(const Place p) {
    this->places[p.identifier] = p;
}

// Shortcut to add multiple places at once, called with an initializer list as argument
// Could most likely be extended using a template to accept any iterable
void Interpreter::addPlaces(const std::initializer_list<Place> places) {
    for(auto &p : places)
        this->addPlace(p);
}

// Shortcut to add multiple transitions at once, same as above
void Interpreter::addTransitions(const std::initializer_list<Transition> transitions) {
    for(auto &t : transitions)
        this->addTransition(t);
}

void Interpreter::delayedFire(Transition &tr, uint32_t delay_ms) {
    // TODO: compensate for the time it takes to start the thread
    // (might not be significant, have to measure that)
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    // TODO: check the boolean guard
    if(tr.canFire()) {
        tr.fire();
        LOG_D("Fired transition %s after %u ms delay", tr.identifier.c_str(), delay_ms);
    } else {
        LOG_I("Ignored %u ms timer for transition %s", delay_ms, tr.identifier.c_str());
    }
}

void Interpreter::doTransitions() {
    // We will store transitions which can be fired here, along with their order
    // After that we sort the array and fire them in order, skipping those that no longer meet the conditions
    std::vector<std::pair<uint32_t, Transition*>> to_fire;

    // If any transitions fired during the last loop, we have to run another check
    uint32_t fire_count;

    // Acquire the transition lock in case any timed transitions want to fire while this is running
    std::lock_guard tr_lock(this->transition_lock);
    
    // RAII magic right there - the lock_guard constructor acquires the lock and the destructor releases it
    // So we effectively lock it for the duration of this scope, C++ does the work for us!
    // Can't decide if this is beautiful or ugly, but it's good practice apparently

    do {
        // Clear the list of transitions to fire and reset the count
        to_fire.clear();
        fire_count = 0;

        // Iterating over a hash map eww
        for(auto &transition : transitions) {
        if(transition.second.canFire()) {
            Transition *t = &transition.second;
            const uint32_t order = transitionOrder[t->identifier];
            to_fire.push_back({order, t});
        }  
        }
        
        // Pair comparison is defined as comparing the first item, then the second.
        // Comparing Transition pointers does not produce meaningful results but
        // the order numbers are unique, so the pointers will never actually be compared
        std::sort(to_fire.begin(), to_fire.end());

        // Evil C++17 pair unpacking
        for(auto &[order, transition] : to_fire) {
            // TODO: Check the boolean guard
            if(transition->canFire() && transition->firesOnEvent(last_input)) {
                if(!transition->isDelayed()) {
                    transition->fire();
                    fire_count++;
                } else {
                    // Create a thread and detach it from the current scope
                    std::thread delay_thread(&Interpreter::delayedFire, this, *transition, transition->getFireCondition()->delayMs);
                    delay_thread.detach();
                }
            }
        }
    } while(fire_count > 0);
}