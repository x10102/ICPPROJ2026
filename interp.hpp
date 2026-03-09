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
        uint32_t max_order;

    public:
        void add_place(Place p);
        void add_transition(Transition t);
        void do_immediate_transitions();
        Interpreter();
        
};

#endif