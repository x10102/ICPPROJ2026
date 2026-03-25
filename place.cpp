// place.cpp - Place class implementation
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>

#include <cstdint>
#include <string>
#include <chrono>

#include "petri.hpp"

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

uint32_t Place::getInitTokens(void) const {
    return initialTokens;
}

void Place::addTokens(const uint32_t token_count) {
    currentTokens += token_count;
    lastChange = duration_cast<milliseconds>(steady_clock::now().time_since_epoch());
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