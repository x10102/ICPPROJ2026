// Petri.hpp - Petri net objects interface
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>

#ifndef _PETRI_H
#define _PETRI_H

#include <cstdint>
#include <string>

class Place {
    // Represents a place in the Petri net
    protected:
        uint32_t initial_tokens;
        uint32_t current_tokens;
        // TODO: Place action??

    public:
        std::string identifier;

    Place(std::string identifier, uint32_t initial_tokens);

    uint32_t get_token_count(void);
    void add_tokens(uint32_t token_count);
    bool remove_tokens(uint32_t token_count);
};

struct TransitionEdge {
    // Represents a directed edge in the Petri net
    Place *from;
    Place *to;
    // Number of tokens consumed in the transition
    uint32_t weight;
};

#endif