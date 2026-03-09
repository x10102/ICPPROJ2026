// petri.hpp - Petri net objects interface
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>

#ifndef _PETRI_H
#define _PETRI_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class Transition;
class Place {
    // Represents a place in the Petri net
    protected:
        uint32_t initialTokens;
        uint32_t currentTokens;
        // TODO: Place action??

    public:
        std::string identifier;

    Place(std::string identifier, uint32_t initial_tokens);
    Place()=default;

    uint32_t getTokenCount(void) const;
    void addTokens(const uint32_t token_count);
    bool removeTokens(const uint32_t token_count);
};

struct TransitionEdge {
    // Represents a directed edge in the Petri net
    Place *place;
    Transition *transition;
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

    Transition(std::string identifier);
    Transition() = default;
    void addEntryEdge(TransitionEdge e);
    void addEntryEdge(Place *from, uint32_t weight);
    void addExitEdge(TransitionEdge e);
    void addExitEdge(Place *to, uint32_t weight);
    void setAction(std::function<void(TransitionEventParams)>);
    void setFireCondition(TransitionFireCondition cond);
    void fire();
    bool canFire();
    bool isDelayed() const;
    bool firesOnEvent(const std::string s) const;
};

#endif