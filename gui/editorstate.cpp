/**
 * @file editorstate.cpp
 * @author Ondřej Turek, xtureko00
 * @brief Implementace JSON serializace / deserializace
 */
#include "editorstate.hpp"
#include "items.hpp"
#include "picojson.h"
#include <cstdint>
#include <iostream>
#include <utility>

// Macro for a picojson value conversion, used very often in this file
#define V(x) picojson::value(x)

using std::make_pair;
using std::pair;
using std::string;

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
    json["initialTokens"] = V(static_cast<int64_t>(this->initial_tokens));
    json["x"] = V(this->x);
    json["y"] = V(this->y);
    return json;
}

picojson::object PetriTransition::json() const {
    picojson::object json;
    json["name"] = V(this->name);
    json["inputEventName"] = V(this->inputEventName);
    json["booleanGuard"] = V(this->booleanGuardMacro);
    json["delayMs"] = V(static_cast<int64_t>(this->delayMs));
    json["x"] = V(this->x);
    json["y"] = V(this->y);
    return json;
}

picojson::object PetriArc::json() const {
    picojson::object json;
    json["place"] = V(this->place->name);
    json["transition"] = V(this->transition->name);
    json["tokenCount"] = V(static_cast<int64_t>(this->tokenCount));
    // TODO: This might be dangerous (moreso during deserialization)
    json["arcType"] = V(static_cast<int64_t>(this->type));
    return json;
}

pair<string, string> PetriNetworkSpec::getArcMapKey(ArcItem *arc) {
    bool sourceIsPlace = (qgraphicsitem_cast<PlaceItem*>(arc->fromItem()) != nullptr);
    std::string placeName;
    std::string transitionName;
    if(sourceIsPlace) {
        placeName = qgraphicsitem_cast<PlaceItem*>(arc->fromItem())->name().toStdString();
        transitionName = qgraphicsitem_cast<TransitionItem*>(arc->toItem())->name().toStdString();
    } else {
        placeName = qgraphicsitem_cast<PlaceItem*>(arc->toItem())->name().toStdString();
        transitionName = qgraphicsitem_cast<TransitionItem*>(arc->fromItem())->name().toStdString();
    }
    return make_pair(placeName, transitionName);
}

void PetriNetworkSpec::addPlace(PetriPlace p) {
    this->places[p.name] = p;
}
void PetriNetworkSpec::addTransition(PetriTransition t) {
    this->transitions[t.name] = t;
}

void PetriNetworkSpec::addArcFromPlace(std::string placeName, std::string transitionName, unsigned int tokenCount) {
    // Not checking for existence here, if they're not in the map then the logic is wrong
    PetriTransition *transition = &transitions.find(transitionName)->second;
    PetriPlace *place = &places.find(placeName)->second;
    PetriArc arc;
    arc.transition = transition;
    arc.place = place;
    arc.tokenCount = tokenCount;
    arc.type = PLACE_TO_TRANSITION;
    arcs.insert({make_pair(place->name, transition->name), arc});
}

void PetriNetworkSpec::renamePlace(string oldName, string newName) {
    PetriPlace *place = getPlace(oldName);
    if(place == nullptr) return;
    std::cout << "Place " << oldName << " found.";
    std::cout.flush();
    // Highly evil map hacking, might be faster than just deleting and reinserting though
    for(auto it = arcs.begin(); it != arcs.end();) {
        if(it->first.first.compare(place->name) == 0) {
            auto editedItem = arcs.extract(it);
            editedItem.key().first = newName;
            arcs.insert(std::move(editedItem));
        } else {
            it++;
        }
    }
    place->name = newName;
}

void PetriNetworkSpec::renameTransition(string oldName, string newName) {
    PetriTransition *tr = getTransition(oldName);
    if(tr == nullptr) return;
    for(auto it = arcs.begin(); it != arcs.end();) {
        if(it->first.second.compare(tr->name) == 0) {
            auto editedItem = arcs.extract(it);
            editedItem.key().second = newName;
            arcs.insert(std::move(editedItem));
        } else {
            it++;
        }
    }
    tr->name = newName;
}


void PetriNetworkSpec::addArcToPlace(std::string placeName, std::string transitionName, unsigned int tokenCount) {
    PetriTransition *transition = &transitions.find(transitionName)->second;
    PetriPlace *place = &places.find(placeName)->second;
    PetriArc arc;
    arc.transition = transition;
    arc.place = place;
    arc.tokenCount = tokenCount;
    arc.type = TRANSITION_TO_PLACE;
    arcs.insert({make_pair(place->name, transition->name), arc});
}

void PetriNetworkSpec::removePlace(std::string name) {
    auto p = places.find(name);
    std::string placeName;
    if(p != places.end()) {
        placeName = p->second.name;
        places.erase(p);
    } else {
        return;
    }
    // Filter the map of arcs, remove any to/from this place
    // would use std::erase_if, but that's C++20
    for(auto it = arcs.begin(); it != arcs.end();) {
        if(it->first.first.compare(placeName) == 0)
            // erase() returns the next valid iterator
            it = arcs.erase(it);
        else
            it++;
    }
}
void PetriNetworkSpec::removeTransition(std::string name) {
    auto t = transitions.find(name);
    std::string transitionName;
    if(t != transitions.end()) {
        transitionName = t->second.name;
        transitions.erase(t);
    } else {
        return;
    }
    // This really should be a utility function
    // but generics in C++ are so awful to write I'm just copying the code
    for(auto it = arcs.begin(); it != arcs.end();) {
        if(it->first.second.compare(transitionName) == 0)
            it = arcs.erase(it);
        else
            it++;
    }
}

PetriArc* PetriNetworkSpec::getArc(ArcItem *arc) {
    auto key = getArcMapKey(arc);
    auto found = arcs.find(key);
    return found == arcs.end() ? nullptr : &found->second; 
}

void PetriNetworkSpec::removeArc(PetriPlace *p, PetriTransition *t) {
    auto key = make_pair(p->name, t->name);
    auto arc = arcs.find(key);
    if(arc == arcs.end()) 
        return;
    arcs.erase(arc);
}

void PetriNetworkSpec::removeArc(ArcItem *arc) {
    auto found = arcs.find(getArcMapKey(arc));
    if(found != arcs.end())
        arcs.erase(found);
}

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
    if(item == this->transitions.end())
        return nullptr;
    return &item->second;
}

std::string PetriNetworkSpec::exportJSON() const {
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
    root["arcs"] = V(arcs);
    root["name"] = V(this->name);
    root["description"] = V(this->description);
    root["variables"] = V(variables);
    root["inputs"] = V(inputs);
    root["outputs"] = V(outputs);

    return V(root).serialize();
}