#ifndef _INTERP_H
#define _INTERP_H

#include "petri.hpp"
#include <string>
#include <unordered_map>
class Interpreter {

    protected:
        std::unordered_map<std::string, Place> places;
        std::unordered_map<std::string, Transition> transitions;

    public:
        void add_place(Place p);
        void add_transition(Transition t);
};

#endif