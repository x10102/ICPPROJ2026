// place.cpp - Place class implementation
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>
// - Adam Šrámek <xsramea00@stud.fit.vutbr.cz>


#include <cstdint>
#include <string>
#include <chrono>

#include "gui/picojson.h"
#include "petri.hpp"

#define V(x) picojson::value(x)

// We have to do this otherwise we will have 200 character long lines
// I LOVE the C++ standard library!
using namespace std::chrono;

Place::Place(std::string identifier, uint32_t initial_tokens) {
    this->identifier = identifier;
    this->initialTokens = initial_tokens;
    this->currentTokens = initial_tokens;
}

uint32_t Place::getTokenCount(void) const {
    return currentTokens;
}

milliseconds Place::getLastChangeTime(void) const {
    return lastChange;
}

std::function<void(void)> Place::getAction(void) const {
    return placeEventAction;
}

uint32_t Place::getInitTokens(void) const {
    return initialTokens;
}

void Place::addTokens(const uint32_t token_count) {
    currentTokens += token_count;
    lastChange = duration_cast<milliseconds>(steady_clock::now().time_since_epoch());
    if(placeEventAction) placeEventAction();
}

bool Place::removeTokens(const uint32_t token_count) {
    // Removes n tokens from the place
    // Returning false if the operation would drop the token count below 0
    // true otherwise 
    if(currentTokens >= token_count) {
        currentTokens -= token_count;
        lastChange = duration_cast<milliseconds>(steady_clock::now().time_since_epoch());
        return true;
    } else {
        return false;
    }
}

void Place::setIdentifier(const std::string identifier) {
    this->identifier = identifier;
}

void Place::setAction(std::function<void(void)> action) {
    this->placeEventAction = action;
}

picojson::object Place::json() {
    picojson::object json;
    json["name"] = V(this->identifier);
    json["currentTokens"] = V(static_cast<double>(this->currentTokens));
    json["initialTokens"] = V(static_cast<double>(this->initialTokens));
    return json;
}