#include "interp.hpp"
#include <cstdint>

// Global variables... unfortunate but can't reasonably do this without them
extern Interpreter *interpreter;

void setHelperInterpreter(Interpreter *itr);
const char* valueof(string input_id);
bool defined(string input_id);
void output(string output_id, string val);
uint32_t tokens(string place_id);
uint64_t elapsed(string place_or_transition_id);
uint64_t now();