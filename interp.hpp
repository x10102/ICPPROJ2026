#ifndef _INTERP_H
#define _INTERP_H

#include "petri.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
class Interpreter {

    protected:
        std::unordered_map<std::string, Place> places;
        std::unordered_map<std::string, Transition> transitions;
        std::unordered_map<std::string, uint32_t> transitionOrder;
        std::unordered_map<std::string, std::string> inputValues;
        std::string last_input;
        uint32_t max_order;

    public:
        void addPlace(Place p);
        void addTransition(Transition t);
        void doTransitions();
        void inputEvent(std::string input, std::string value);
        bool inputDefined(std::string input);
        Interpreter();

};

#endif