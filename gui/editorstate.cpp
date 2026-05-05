#include "editorstate.hpp"
#include "picojson.h"

picojson::object PetriPlace::json() {
    picojson::object json;
    json["name"] = picojson::value(this->name);
    json["placeAction"] = picojson::value(this->placeActionMacro);
    json["initialTokens"] = picojson::value(static_cast<double>(this->initial_tokens));
    return json;
}

picojson::object PetriTransition::json() {
    picojson::object json;
    json["name"] = picojson::value(this->name);
    json["inputEventName"] = picojson::value(this->inputEventName);
    json["booleanGuard"] = picojson::value(this->booleanGuardMacro);
    json["delayMs"] = picojson::value(static_cast<double>(this->delayMs));
    return json;
}

void PetriNetworkSpec::addPlace(PetriPlace p) {
    this->places[p.name] = p;
}
void PetriNetworkSpec::addTransition(PetriTransition t) {
    this->transitions[t.name] = t;
}

void PetriNetworkSpec::addArcFromPlace(PetriPlace p, PetriTransition t) {
}
void PetriNetworkSpec::addArcToPlace(PetriPlace p, PetriTransition t) {}

void PetriNetworkSpec::removePlace(std::string name) {}
void PetriNetworkSpec::removeTransition(std::string name) {}
void PetriNetworkSpec::removeArc(PetriPlace *p, PetriTransition *t) {}

PetriPlace* PetriNetworkSpec::getPlace(std::string name) {
    auto item = this->places.find(name);
    if(item == this->places.end())
        return nullptr;
    return &item->second;
}

void PetriNetworkSpec::exportJSON() {
    picojson::object root;
    picojson::object places;
    picojson::object transitions;

    for(auto &place : this->places) {
        places[place.second.name] = picojson::value(place.second.json());
    }

    for (auto &transition : this->transitions) {
        transitions[transition.second.name] = picojson::value(transition.second.json());
    }

    root["places"] = picojson::value(places);
    root["transitions"] = picojson::value(transitions);

    std::cout << picojson::value(root).serialize() << std::endl;
    
}