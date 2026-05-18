/**
 * @file interp.cpp
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

        [[nodiscard]] Place* createPlace(string identifier, uint32_t initial_tokens);
        Place* getPlace(string identifier);
        bool renamePlace(const string &oldId, const string &newId);
        bool removePlace(const string &identifier);
        [[nodiscard]] Transition* createTransition(string identifier);
        bool renameTransition(const string &oldId, const string &newId);
        bool removeTransition(const string &identifier);
        void doTransitions(bool all = true);
        void inputEvent(const std::string input, const std::string value);
        virtual void outputEvent(const std::string output, const std::string value);
        void waitForAllTimers();
        void terminate();
        virtual void printState();
        bool inputDefined(const std::string input);
        string *lastInputValue(const std::string input);

        // Functions for clearing the internal "queues" of events and fired transitions
        void clearFired();
        void clearEvents();
        void run(uint16_t port);
        
        picojson::object json();

        Interpreter();

};

#endif