/**
 * @file editorstate.hpp
 * @author Ondřej Turek, xtureko00
 * @brief Rozhraní JSON serializace / deserializace
 */
#ifndef _EDITORSTATE_HPP
#define _EDITORSTATE_HPP

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "items.hpp"
#include "picojson.h"
#include "variableeditor.hpp"

enum ArcType {
    PLACE_TO_TRANSITION,
    TRANSITION_TO_PLACE
};

struct PetriPlace {
    std::string name;
    uint32_t initial_tokens;
    std::string placeActionMacro;
    double x = 0.0;
    double y = 0.0;

    picojson::object json() const;
    PetriPlace();
};

struct PetriTransition {
    std::string name;
    std::string inputEventName;
    std::string booleanGuardMacro;
    std::string tranActionMacro;
    uint32_t delayMs;
    double x = 0.0;
    double y = 0.0;

    picojson::object json() const;
    PetriTransition();
};

struct PetriArc {
    // Use regular pointers here, the Place references are not owned
    PetriPlace *place;
    PetriTransition *transition;
    uint32_t tokenCount;
    ArcType type;

    picojson::object json() const;
    PetriArc();
};

// No encapsulation here :(
// Making the internal attributes private would make our life really difficult
// when writing the interpreter generator
struct PetriNetworkSpec {

    void addPlace(PetriPlace p);
    void addTransition(PetriTransition t);
    void addArcFromPlace(std::string placeName, std::string transitionName, unsigned int tokenCount);
    void addArcToPlace(std::string placeName, std::string transitionName, unsigned int tokenCount);
    void addInput(std::string inputName);
    void addOutput(std::string outputName);
    void setNetworkName(std::string name);
    void setDescription(std::string description);

    void setVariableMap(VariableMap *vars);

    void removePlace(std::string name);
    void removeTransition(std::string name);
    void removeArc(PetriPlace *p, PetriTransition *t);
    void removeArc(ArcItem *arc);
    void removeInput(std::string inputName);
    void removeOutput(std::string outputName);

    std::string exportJSON() const;
    bool loadJSON(std::string jsonString);

    PetriPlace* getPlace(std::string name);
    PetriTransition* getTransition(std::string name);
    PetriArc* getArc(ArcItem *arc);

    void renamePlace(std::string oldName, std::string newName);
    void renameTransition(std::string oldName, std::string newName);

    std::string name;
    std::string description;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::map<std::string, PetriPlace> places;
    std::map<std::string, PetriTransition> transitions;
    std::map<std::pair<std::string, std::string>, PetriArc> arcs;

    private:
        VariableMap *variables = nullptr;
        std::pair<std::string, std::string> getArcMapKey(ArcItem *arc);
};
#endif