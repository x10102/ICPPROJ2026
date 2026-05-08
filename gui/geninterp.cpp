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
    this->generatedBuffer << "TRANSITION(";
    this->generatedBuffer << t->name << ", \"" << t->name << "\");" << std::endl;
    if((t->delayMs != 0 || !t->inputEventName.empty())) {
        if(!t->booleanGuardMacro.empty()) {
            this->generatedBuffer << "CONDITION_EXPR(" << t->name;
            this->generatedBuffer << "\"" << t->inputEventName << "\", ";
            this->generatedBuffer << t->delayMs << ", ";
            this->generatedBuffer << t->booleanGuardMacro << ");" << std::endl;
        } else {
            this->generatedBuffer << "CONDITION(" << t->name;
            this->generatedBuffer << "\"" << t->inputEventName << "\", ";
            this->generatedBuffer << t->delayMs << ");" << std::endl;
        }
    }
    
}

void InterpreterGenerator::emitArc(const PetriArc *a) {
    if(a->type == PLACE_TO_TRANSITION) {
        this->generatedBuffer << "ENTRY_EDGE(" << a->place->name << ", ";
        this->generatedBuffer << a->transition->name << ", ";
        this->generatedBuffer << a->tokenCount << ");" << std::endl;
    } else if(a->type == TRANSITION_TO_PLACE) {
        this->generatedBuffer << "EXIT_EDGE(" << a->transition->name << ", ";
        this->generatedBuffer << a->place->name << ", ";
        this->generatedBuffer << a->tokenCount << ");" << std::endl;
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