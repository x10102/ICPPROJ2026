// interp.hpp - interpreter interface
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>

#ifndef _INTERP_H
#define _INTERP_H

#include "petri.hpp"
#include <cstdint>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <mutex>
class Interpreter {

    protected:
        std::unordered_map<std::string, Place> places;
        std::unordered_map<std::string, Transition> transitions;
        std::unordered_map<std::string, uint32_t> transitionOrder;
        std::unordered_map<std::string, std::string> inputValues;
        std::string last_input;
        std::mutex transition_lock;
        uint32_t max_order;

        void delayedFire(Transition &tr, uint32_t delay_ms);

    public:
        void addPlace(const Place p);
        void addPlaces(const std::initializer_list<Place> places);
        void addTransition(const Transition t);
        void addTransitions(const std::initializer_list<Transition> transitions);
        void doTransitions();
        void inputEvent(const std::string input, const std::string value);
        bool inputDefined(const std::string input);
        Interpreter();

};

#endif