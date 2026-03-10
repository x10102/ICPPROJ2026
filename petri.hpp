// petri.hpp - Petri net objects interface
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>

#ifndef _PETRI_H
#define _PETRI_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <chrono>

class Transition;
class Place {
    // Represents a place in the Petri net
    protected:
        uint32_t initialTokens;
        uint32_t currentTokens;
        std::chrono::milliseconds lastChange;
        // TODO: Place action??

    public:
        std::string identifier;

    Place(std::string identifier, uint32_t initial_tokens);
    Place()=default;

    uint32_t getTokenCount(void) const;
    uint32_t getInitTokens(void) const;
    std::chrono::milliseconds getLastChangeTime(void) const;
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

struct TransitionFireCondition {
    uint32_t delayMs;
    std::string inputEventName;
    std::function<bool(void)> boolExpr;
};

class Transition {
    
    protected:
        std::vector<TransitionEdge> enterEdges;
        std::vector<TransitionEdge> exitEdges;
        std::function<void(void)> transitionEventAction;
        TransitionFireCondition fireCondition;

    public:
        std::string identifier;

    Transition(std::string identifier);
    Transition(void) = default;
    void addEntryEdge(TransitionEdge e);
    void addEntryEdge(Place *from, uint32_t weight);
    void addExitEdge(TransitionEdge e);
    void addExitEdge(Place *to, uint32_t weight);
    void setAction(std::function<void(void)> action);
    void setFireCondition(TransitionFireCondition cond);
    TransitionFireCondition* getFireCondition(void);
    void fire(void);
    bool canFire(void);
    bool isDelayed(void) const;
    bool firesOnEvent(const std::string s) const;
    bool checkGuard(void);
};

#endif