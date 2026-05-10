#include "geninterp.hpp"
#include "editorstate.hpp"
#include <ostream>

void InterpreterGenerator::emitPlace(const PetriPlace *p) {
    this->generatedBuffer << "PLACE(";
    this->generatedBuffer << p->name << ", \"" << p->name << "\", ";
    this->generatedBuffer << p->initial_tokens;
    this->generatedBuffer << ");" << std::endl;
    if(!p->placeActionMacro.empty()) {
        this->generatedBuffer << "ACTION(";
        this->generatedBuffer << p->name << ", ";
        this->generatedBuffer << p->placeActionMacro;
        this->generatedBuffer << ")" << std::endl;
    }
}

void InterpreterGenerator::emitTransition(const PetriTransition *t) {
    generatedBuffer << "TRANSITION(";
    generatedBuffer << t->name << ", \"" << t->name << "\");" << std::endl;
    if((t->delayMs != 0 || !t->inputEventName.empty())) {
        if(!t->booleanGuardMacro.empty()) {
            generatedBuffer << "CONDITION_EXPR(" << t->name;
            generatedBuffer << "\"" << t->inputEventName << "\", ";
            generatedBuffer << t->delayMs << ", ";
            generatedBuffer << t->booleanGuardMacro << ");" << std::endl;
        } else {
            generatedBuffer << "CONDITION(" << t->name;
            generatedBuffer << "\"" << t->inputEventName << "\", ";
            generatedBuffer << t->delayMs << ");" << std::endl;
        }
    }
    if(!t->tranActionMacro.empty()) {
        generatedBuffer << "ACTION(" << t->name << ", " << t->tranActionMacro << ");" << std::endl;
    }
    
}

void InterpreterGenerator::emitArc(const PetriArc *a) {
    if(a->type == PLACE_TO_TRANSITION) {
        generatedBuffer << "ENTRY_EDGE(" << a->place->name << ", ";
        generatedBuffer << a->transition->name << ", ";
        generatedBuffer << a->tokenCount << ");" << std::endl;
    } else if(a->type == TRANSITION_TO_PLACE) {
        generatedBuffer << "EXIT_EDGE(" << a->transition->name << ", ";
        generatedBuffer << a->place->name << ", ";
        generatedBuffer << a->tokenCount << ");" << std::endl;
    }
}

bool InterpreterGenerator::generateMain(const PetriNetworkSpec *spec) {
    std::cout << "Generated main:" << std::endl;
    generatedBuffer.str("");
    for(const auto &p : spec->places)
        this->emitPlace(&p.second);
    for(const auto &tr : spec->transitions)
        this->emitTransition(&tr.second);
    for(const auto &arc : spec->arcs)
        this->emitArc(&arc.second); 
    std::cout << generatedBuffer.str();
    return true;
}