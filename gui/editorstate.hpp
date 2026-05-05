#ifndef _EDITORSTATE_HPP
#define _EDITORSTATE_HPP

#include <cstdint>
#include <map>
#include <string>

#include "picojson.h"

enum ArcType {
    PLACE_TO_TRANSITION,
    TRANSITION_TO_PLACE
};

struct PetriPlace {
    std::string name;
    uint32_t initial_tokens;
    std::string placeActionMacro;

    picojson::object json();
};

struct PetriTransition {
    std::string name;
    std::string inputEventName;
    std::string booleanGuardMacro;
    uint32_t delayMs;

    picojson::object json();
};

struct PetriArc {
    // Use regular pointers here, the Place references are not owned
    PetriPlace *place;
    PetriTransition *transition;
    uint32_t tokenCount;
    ArcType type;

    picojson::object json();
};

class PetriNetworkSpec {

    public:
    void addPlace(PetriPlace p);
    void addTransition(PetriTransition t);
    void addArcFromPlace(PetriPlace p, PetriTransition t);
    void addArcToPlace(PetriPlace p, PetriTransition t);

    void removePlace(std::string name);
    void removeTransition(std::string name);
    void removeArc(PetriPlace *p, PetriTransition *t);

    void exportJSON();

    PetriPlace* getPlace(std::string name);

    private:
    std::map<std::string, PetriPlace> places;
    std::map<std::string, PetriTransition> transitions;
    std::map<std::string, PetriArc*> arcs;
};
#endif