// place.cpp - Place class implementation
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>

#include <cstdint>
#include <string>

#include "petri.hpp"

Place::Place(std::string identifier, uint32_t initial_tokens) {
    this->identifier = identifier;
    this->initialTokens = initial_tokens;
    this->currentTokens = initial_tokens;
}

uint32_t Place::getTokenCount(void) const {
    return currentTokens;
}

void Place::addTokens(const uint32_t token_count) {
    currentTokens += token_count;
}

bool Place::removeTokens(const uint32_t token_count) {
    // Removes n tokens from the place
    // Returning false if the operation would drop the token count below 0
    // true otherwise 
    if(currentTokens >= token_count) {
        currentTokens -= token_count;
        return true;
    } else {
        return false;
    }
}
