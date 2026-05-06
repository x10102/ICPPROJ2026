#include "geninterp.hpp"
#include "editorstate.hpp"
#include <ostream>

void InterpreterGenerator::emitPlace(PetriPlace *p) {
    this->generatedBuffer << "PLACE(";
    this->generatedBuffer << p->name << ", " << p->name << ", ";
    this->generatedBuffer << p->initial_tokens;
    this->generatedBuffer << ");" << std::endl;
    this->generatedBuffer << "ACTION(";
    this->generatedBuffer << p->name << ", ";
    this->generatedBuffer << p->placeActionMacro;
    this->generatedBuffer << ")" << std::endl;
}

void InterpreterGenerator::emitTransition(PetriTransition *t) {
    this->generatedBuffer << "TRANSITION(";
    this->generatedBuffer << t->name << ", " << t->name << ");" << std::endl;
    this->generatedBuffer << "CONDITION_EXPR(" << t->name;
    this->generatedBuffer << t->inputEventName << ", ";
    this->generatedBuffer << t->delayMs << ", ";
    this->generatedBuffer << t->booleanGuardMacro << ");" << std::endl;
}

void InterpreterGenerator::emitArc(PetriArc *a) {
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