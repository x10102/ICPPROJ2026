// Petri.hpp - Petri net objects interface
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>

#ifndef _PETRI_H
#define _PETRI_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class Place {
    // Represents a place in the Petri net
    protected:
        uint32_t initialTokens;
        uint32_t currentTokens;
        // TODO: Place action??

    public:
        std::string identifier;

    Place(std::string identifier, uint32_t initial_tokens);

    uint32_t get_token_count(void);
    void add_tokens(uint32_t token_count);
    bool remove_tokens(uint32_t token_count);
};

struct TransitionEdge {
    // Represents a directed edge in the Petri net
    Place *from;
    Place *to;
    // Number of tokens consumed in the transition
    uint32_t weight;
};

struct TransitionEventParams {
    // TODO:
};

struct TransitionFireCondition {
    uint32_t delayMs;
    std::string inputEventName;
    // TODO: Bool expression - variadic std::function??
    // No clue how we are supposed to do that
    // Possibly function that gets passed a struct with the current interpreter state
};

class Transition {
    
    protected:
        std::vector<TransitionEdge> enterEdges;
        std::vector<TransitionEdge> exitEdges;
        std::function<void(TransitionEventParams)> transitionEventAction;
        TransitionFireCondition fireCondition; // Maybe this should be a unique_ptr?

    public:
        std::string identifier;

    void addEntryEdge(TransitionEdge e);
    void addExitEdge(TransitionEdge e);
    void setAction(std::function<void(TransitionEventParams)>);
    void setFireCondition(TransitionFireCondition cond);
};

#endif