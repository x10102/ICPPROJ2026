// Petri.cpp - Petri net objects implementation
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>

#include <cstdint>
#include <string>

#include "petri.hpp"

// TODO: Might have to split this into multiple files depending on how big it gets

// Place functions

Place::Place(std::string identifier, uint32_t initial_tokens) {
    this->identifier = identifier;
    this->initial_tokens = initial_tokens;
}

uint32_t Place::get_token_count(void) {
    return current_tokens;
}

void Place::add_tokens(uint32_t token_count) {
    current_tokens += token_count;
}

bool Place::remove_tokens(uint32_t token_count) {
    // Removes n tokens from the place
    // Returning false if the operation would drop the token count below 0
    // true otherwise 
    if(current_tokens >= token_count) {
        current_tokens -= token_count;
        return true;
    } else {
        return false;
    }
}
