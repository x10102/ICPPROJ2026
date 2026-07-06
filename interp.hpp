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
        /** A mapping of the place identifiers to their instances */
        unordered_map<string, unique_ptr<Place>> places;
        /** A mapping of transition identifiers to their instances */
        unordered_map<string, unique_ptr<Transition>> transitions;
        /** A mapping of transition identifiers to the index according to which the execution is ordered */
        unordered_map<string, uint32_t> transitionOrder;
        /** A mapping of input names to their last recorded value */
        unordered_map<string, string> inputValues;
        /** A vector storing thread objects of transition timers */
        vector<thread> timerThreads;
        /** The value of the last input */
        string last_input;
        mutex transition_lock;
        uint32_t max_order;
        bool exiting;

        /** 
        * Thread entry point for delayed transitions
        * \param tr Pointer to the transition to fire
        * \param delay_ms The transition's delay
        */ 
        void delayedFire(Transition *tr, uint32_t delay_ms);

    public:
        Place* createPlace(string identifier, uint32_t initial_tokens);
        Place* getPlace(string identifier);
        Transition* createTransition(string identifier);
        void doTransitions();
        void inputEvent(const std::string input, const std::string value);
        void outputEvent(const std::string output, const std::string value);
        void waitForAllTimers();
        void terminate();
        void printState();
        bool inputDefined(const std::string input);
        std::string *lastInputValue(const std::string input);
        Interpreter();

};

#endif