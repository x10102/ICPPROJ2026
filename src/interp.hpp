/**
 * @file interp.hpp
 * @author Ondřej Turek, xtureko00
 * @brief Interpreter engine interface
 */

#ifndef _INTERP_H
#define _INTERP_H

#define NETWORK_BUFFER_SIZE 8192

#include "petri.hpp"
#include "gui/picojson.h"
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <mutex>
#include <vector>

struct Variable {
    std::string type;
    std::string name;
    std::string value;
};

using namespace std;
class Interpreter {

    protected:
        unordered_map<string, string> inputValues;
        unordered_map<string, string> outputValues;
        vector<Variable> variables;
        unordered_map<string, unique_ptr<Place>> places;
        unordered_map<string, unique_ptr<Transition>> transitions;
        vector<OutputEvent> events;
        vector<string> firedLastStep;

        unordered_map<string, uint32_t> transitionOrder;
        vector<thread> timerThreads;
        string last_input;
        mutex transition_lock;
        uint32_t max_order;
        bool exiting;

        void delayedFire(Transition *tr, uint32_t delay_ms);

    public:

        [[nodiscard]] Place* createPlace(string identifier, uint32_t initial_tokens); ///< Creates a place in the interpreter
        Place* getPlace(string identifier); ///< Retrieves a place in the interpreter
        bool renamePlace(const string &oldId, const string &newId); ///< Renames a place in the interpreter
        bool removePlace(const string &identifier); ///< Deletes a place in the interpreter
        [[nodiscard]] Transition* createTransition(string identifier); ///< Creates a transition in the interpreter
        bool renameTransition(const string &oldId, const string &newId);
        bool removeTransition(const string &identifier);
        void doTransitions(bool all = true); ///< Processes all transitions which can be fired
        void inputEvent(const std::string input, const std::string value); ///< Sets an input to a value
        virtual void outputEvent(const std::string output, const std::string value); ///< Generates an output event
        void waitForAllTimers(); ///< Waits for all timed transitions to expire
        void terminate(); ///< Exits the interpreter
        virtual void printState(); ///< Prints the current markings to the standard output
        bool inputDefined(const std::string input); ///< Checks if an input is defined
        string *lastInputValue(const std::string input); ///< Retrieves the last known value of an input

        // Functions for clearing the internal "queues" of events and fired transitions
        void clearFired();
        void clearEvents();
        void run(uint16_t port);
        
        picojson::object json();

        Interpreter();

};

#endif