// interp.cpp - interpreter implementation
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>
// - Adam Šrámek <xsramea00@stud.fit.vutbr.cz>

#include "petri.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include "interp.hpp"
#include "debug.hpp"
#include "gui/picojson.h"

using namespace std;

Interpreter::Interpreter() {
    this->max_order = 0;
    this->exiting = false;
}

picojson::object Interpreter::json() {
    picojson::object json;
    picojson::array aPlaces;
    // We don't really need to send the transitions, they don't hold any state
    // and the editor already knows about them
    //picojson::array aTransitions;
    picojson::array firedLast;

    // TODO: timers
    for(auto &p : this->places)
        aPlaces.push_back(picojson::value(p.second->json()));
    /*for(auto &t : this->transitions)
        aTransitions.push_back(picojson::value(t.second->json()));*/
    for(auto &f : this->firedLastStep)
        firedLast.push_back(picojson::value(f));
    json["places"] = picojson::value(aPlaces);
    //json["transitions"] = picojson::value(aTransitions);
    json["fired"] = picojson::value(firedLast);
    return json;
}

// Triggers an input event -
// - Sets the appropriate input to the value passed
// - Updates the last input name
// - Triggers transition processing
void Interpreter::inputEvent(const std::string input, const std::string value) {
    if(this->exiting) {
        LOG_I("Ignoring input event %s while exiting", input.c_str());
        return;
    }
    LOG_D("Input event: %s=%s", input.c_str(), value.c_str());
    this->inputValues[input] = value;
    this->last_input = input;
    this->doTransitions();
}

void Interpreter::outputEvent(string output, string value) {
    // TODO: This should at least send the output event to the editor
    // For now we just log it
    LOG_I("OUTPUT_EVENT: %s=%s", output.c_str(), value.c_str());
}

bool Interpreter::inputDefined(const std::string input) {
    return this->inputValues.count(input) != 0;
}

string *Interpreter::lastInputValue(string input) {
    try {
        return &this->inputValues.at(input);
    } catch(const out_of_range &e) {
        return nullptr;
    }
}

void Interpreter::printState() {
    //LOG_T("Print state");
    ostringstream oss;
    oss << setfill('=') << setw(38) << left << "PLACES" << endl << endl << setfill(' ');
    for(auto &place : this->places) {
        oss << left << setw(20) << place.first;
        oss << "ITOK: " << setw(2) << place.second->getInitTokens() << ' '
        << "CTOK: " << setw(2) << place.second->getTokenCount() << endl;
    }
    oss << endl << endl;
    cout << oss.str();
}

// Terminate the interpreter - will ignore all input events and wait for all timer threads to exit
void Interpreter::terminate() {
    this->exiting = true;
    LOG_I("Exit request received");
    LOG_I("Waiting for all threads to terminate...");
    this->waitForAllTimers();
}

// Creates a transition and returns a pointer to it
// !IMPORTANT: Transitions added first have priority in conflicts
Transition* Interpreter::createTransition(string identifier) {
    auto t = make_unique<Transition>(identifier);
    Transition *ptr = t.get();
    transitions.emplace(identifier, std::move(t));
    this->transitionOrder[identifier] = this->max_order++;
    return ptr;
}

bool Interpreter::removeTransition(const string &identifier) {
    transitionOrder.erase(identifier);
    return transitions.erase(identifier) > 0;
}

// Renames transition, ie. changes identifier of transition
bool Interpreter::renameTransition(const string &oldId, const string &newId) {
    auto it = transitions.find(oldId);
    if (it == transitions.end() || transitions.count(newId))
        return false;
    it->second->setIdentifier(newId);
    transitions.emplace(newId, std::move(it->second));
    transitions.erase(it);
    return true;
}

// Creates a place
Place* Interpreter::createPlace(string identifier, uint32_t initial_tokens) {
    auto p = make_unique<Place>(identifier, initial_tokens);
    Place *ptr = p.get();
    places.emplace(identifier, std::move(p));
    return ptr;
}

bool Interpreter::removePlace(const string &identifier) {
    return places.erase(identifier) > 0;
}

// Renames place, ie. changes identifier of place
bool Interpreter::renamePlace(const string &oldId, const string &newId) {
    auto it = places.find(oldId);
    if (it == places.end() || places.count(newId))
        return false;
    it->second->setIdentifier(newId);
    places.emplace(newId, std::move(it->second));
    places.erase(it);
    return true;
}

// Returns a pointer to a place with the specified identifier
// or nullptr if it doesn't exist
Place* Interpreter::getPlace(string identifier) {
    try {
        return this->places.at(identifier).get();
    } catch(const out_of_range &e) {
        return nullptr;
    }
}

// Thread entry point for delayed transitions
void Interpreter::delayedFire(Transition *tr, uint32_t delay_ms) {
    // TODO: compensate for the time it takes to start the thread
    // (might not be significant, have to measure that)
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    if(exiting) {
        LOG_I("Ignoring delayed fire of %s while exiting", tr->identifier.c_str());
        return;
    }
    std::lock_guard guard(this->transition_lock);
    if(tr->canFire() && tr->checkGuard()) {
        this->firedLastStep.push_back(tr->identifier);
        tr->fire();
        LOG_D("Fired transition %s after %u ms delay", tr->identifier.c_str(), delay_ms);
    } else {
        LOG_I("Ignored %u ms timer for transition %s", delay_ms, tr->identifier.c_str());
    }
}

void Interpreter::waitForAllTimers() {
    for(auto thr = timerThreads.begin(); thr < timerThreads.end(); thr++) {
        thr->join();
        timerThreads.erase(thr);
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
            if(transition.second->canFire()) {
                Transition *t = transition.second.get();
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

            // Don't even bother with anything else if the transition
            // doesn't fire on this event or boolean guard evals to 'false'
            if(!transition->firesOnEvent(last_input) || !transition->checkGuard())
                continue;

            if(!transition->isDelayed()) {
                this->firedLastStep.push_back(transition->identifier);
                transition->fire();
                fire_count++;
            } else {
                // Make the transition into a regular pointer, the compiler is not happy with the unpacked one
                Transition *tr = transition;
                // Create a thread and detach it from the current scope
                uint32_t delay = tr->getFireCondition()->delayMs;
                LOG_D("Schedule transition %s after %u ms", tr->identifier.c_str(), delay);
                // No easy way to call an instance method as a thread start other than this
                // TODO: In the final interpreter, it would make more sense to detach the thread here (or use futures)
                auto thr = std::thread([this, delay, tr]() {delayedFire(tr, delay);});
                // Threads aren't copyable - move it into the vector
                timerThreads.emplace_back(std::move(thr));
                
            }
        }
    } while(fire_count > 0);
}

OutputEvent Interpreter::getLastOutputEvent() {
    // TODO: Undefined b. on empty vector
    auto last = this->events.back();
    this->events.pop_back();
    return last;
}

void Interpreter::clearFired() {
    this->firedLastStep.clear();
}