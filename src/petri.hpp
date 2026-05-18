// petri.hpp - Petri net objects interface
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>
// - Adam Šrámek <xsramea00@stud.fit.vutbr.cz>


#ifndef _PETRI_H
#define _PETRI_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <chrono>
#include "gui/picojson.h"

class Transition;
class Place {
    // Represents a place in the Petri net
    protected:
        uint32_t initialTokens;
        uint32_t currentTokens;
        std::chrono::milliseconds lastChange;
        std::function<void(void)> placeEventAction; // Maybe not lambda??


    public:
        std::string identifier;

    Place(std::string identifier, uint32_t initial_tokens);
    Place()=default;

    uint32_t getTokenCount(void) const;
    uint32_t getInitTokens(void) const;
    std::function<void(void)> getAction(void) const;
    std::chrono::milliseconds getLastChangeTime(void) const;
    void addTokens(const uint32_t token_count);
    bool removeTokens(const uint32_t token_count);
    void setIdentifier(const std::string identifier);
    void setAction(std::function<void(void)> action);
    picojson::object json();
};

struct TransitionEdge {
    // Represents a directed edge in the Petri net
    Place *place;
    Transition *transition;
    // Number of tokens consumed in the transition
    uint32_t weight;

    picojson::object json();
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
    void setIdentifier(const std::string identifier);
    const std::vector<TransitionEdge>& getEntryEdges() const;
    const std::vector<TransitionEdge>& getExitEdges()  const;
    void setEntryEdgeWeight(Place *from, uint32_t weight);
    void setExitEdgeWeight(Place *to,   uint32_t weight);
    bool removeEntryEdge(Place *from);
    bool removeExitEdge(Place *to);

    picojson::object json();
};

struct OutputEvent {
    std::string outputId;
    std::string outputVal;
    long timestamp;

    picojson::object json();
};

#endif