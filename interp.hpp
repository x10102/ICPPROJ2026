// interp.hpp - interpreter interface
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>

#ifndef _INTERP_H
#define _INTERP_H

#include "petri.hpp"
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <mutex>

using namespace std;
class Interpreter {

    protected:
        unordered_map<string, unique_ptr<Place>> places;
        unordered_map<string, unique_ptr<Transition>> transitions;
        unordered_map<string, uint32_t> transitionOrder;
        unordered_map<string, string> inputValues;
        vector<thread> timerThreads;
        string last_input;
        mutex transition_lock;
        uint32_t max_order;
        bool exiting;

        void delayedFire(Transition *tr, uint32_t delay_ms);

    public:
        Place* createPlace(string identifier, uint32_t initial_tokens);
        void addPlace(unique_ptr<Place> p);
        void addPlaces(const std::initializer_list<Place> places);
        Transition* createTransition(string identifier);
        void addTransition(unique_ptr<Transition> t);
        void addTransitions(const std::initializer_list<Transition> transitions);
        void doTransitions();
        void inputEvent(const std::string input, const std::string value);
        void waitForAllTimers();
        void terminate();
        void printState();
        bool inputDefined(const std::string input);
        Interpreter();

};

#endif