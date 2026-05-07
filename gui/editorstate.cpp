#include "editorstate.hpp"
#include "picojson.h"

// Macro for a picojson value conversion, used very often in this file
#define V(x) picojson::value(x)

// TODO: Parametric constructor
PetriPlace::PetriPlace() {
    this->initial_tokens = 0;
}

PetriTransition::PetriTransition() {
    this->delayMs = 0;
}

PetriArc::PetriArc() {
    this->tokenCount = 0;
}

picojson::object PetriPlace::json() const {
    picojson::object json;
    json["name"] = V(this->name);
    json["placeAction"] = V(this->placeActionMacro);
    json["initialTokens"] = V(static_cast<double>(this->initial_tokens));
    return json;
}

picojson::object PetriTransition::json() const {
    picojson::object json;
    json["name"] = V(this->name);
    json["inputEventName"] = V(this->inputEventName);
    json["booleanGuard"] = V(this->booleanGuardMacro);
    json["delayMs"] = V(static_cast<double>(this->delayMs));
    return json;
}

picojson::object PetriArc::json() const {
    picojson::object json;
    json["place"] = V(this->place->name);
    json["transition"] = V(this->transition->name);
    json["tokenCount"] = V(static_cast<double>(this->tokenCount));
    // TODO: This might be dangerous (moreso during deserialization)
    json["arcType"] = V(static_cast<double>(this->type));
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

void PetriNetworkSpec::updatePlace(const std::string& oldName, PetriPlace place) {
    if (oldName != place.name) {
        places.erase(oldName);
    }
    places[place.name] = place;
}

void PetriNetworkSpec::updateTransition(const std::string& oldName, PetriTransition transition) {
    if (oldName != transition.name) {
        transitions.erase(oldName);
    }
    transitions[transition.name] = transition;
}

void PetriNetworkSpec::removePlace(std::string name) {}
void PetriNetworkSpec::removeTransition(std::string name) {}
void PetriNetworkSpec::removeArc(PetriPlace *p, PetriTransition *t) {}

void PetriNetworkSpec::addInput(std::string inputName) {
    this->inputs.push_back(inputName);
}
void PetriNetworkSpec::addOutput(std::string outputName) {
    this->outputs.push_back(outputName);
}
void PetriNetworkSpec::addVariableDef(std::string definition) {
    this->variables.push_back(definition);
}
void PetriNetworkSpec::setNetworkName(std::string name) {
    this->name = name;
}
void PetriNetworkSpec::setDescription(std::string description) {
    this->description = description;
}

PetriPlace* PetriNetworkSpec::getPlace(std::string name) {
    const auto item = this->places.find(name);
    if(item == this->places.end())
        return nullptr;
    return &item->second;
}

PetriTransition* PetriNetworkSpec::getTransition(std::string name) {
    const auto item = this->transitions.find(name);
    if (item == this->transitions.end()) {
        return nullptr;
    }
    return &item->second;
}

void PetriNetworkSpec::exportJSON() const {
    picojson::object root;
    picojson::object places;
    picojson::object transitions;
    picojson::array arcs;
    picojson::array variables;
    picojson::array inputs;
    picojson::array outputs;

    // Convert places to picojson value types
    for(auto &place : this->places) {
        places[place.second.name] = V(place.second.json());
    }

    // Same for transitions, variables, etc.
    for (const auto &transition : this->transitions)
        transitions[transition.second.name] = V(transition.second.json());

    for(const auto &var : this->variables)
        variables.push_back(V(var));

    for(const auto &in : this->inputs)
        inputs.push_back(V(in));

    for(const auto &out : this->outputs)
        outputs.push_back(V(out));

    for(const auto &arc : this->arcs)
        arcs.push_back(V(arc.second.json()));

    root["places"] = V(places);
    root["transitions"] = V(transitions);
    root["name"] = V(this->name);
    root["description"] = V(this->description);
    root["variables"] = V(variables);
    root["inputs"] = V(inputs);
    root["outputs"] = V(outputs);

    std::cout << V(root).serialize() << std::endl;
    
}