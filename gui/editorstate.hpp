#ifndef _EDITORSTATE_HPP
#define _EDITORSTATE_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "picojson.h"

enum ArcType {
    PLACE_TO_TRANSITION,
    TRANSITION_TO_PLACE
};

struct PetriPlace {
    std::string name;
    uint32_t initial_tokens;
    std::string placeActionMacro;

    picojson::object json() const;
    PetriPlace();
};

struct PetriTransition {
    std::string name;
    std::string inputEventName;
    std::string booleanGuardMacro;
    std::string transitionActionMacro;
    uint32_t delayMs;

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
    void addArcFromPlace(PetriPlace p, PetriTransition t);
    void addArcToPlace(PetriPlace p, PetriTransition t);
    void addInput(std::string inputName);
    void addOutput(std::string outputName);
    void addVariableDef(std::string definition);
    void setNetworkName(std::string name);
    void setDescription(std::string description);

    void updatePlace(const std::string& oldName, PetriPlace place);
    void updateTransition(const std::string& oldName, PetriTransition transition);

    void removePlace(std::string name);
    void removeTransition(std::string name);
    void removeArc(PetriPlace *p, PetriTransition *t);
    void undefVariable(std::string name);
    void removeInput(std::string inputName);
    void removeOutput(std::string outputName);

    void exportJSON() const;

    PetriPlace* getPlace(std::string name);
    PetriTransition* getTransition(std::string name);
    std::string name;
    std::string description;
    std::vector<std::string> variables;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::map<std::string, PetriPlace> places;
    std::map<std::string, PetriTransition> transitions;
    std::map<std::string, PetriArc> arcs;
};
#endif