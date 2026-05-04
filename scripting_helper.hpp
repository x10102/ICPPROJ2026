// scripting_helper.hpp - Inscription language helpers interface
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>
#ifndef _SHELPER_H
#define _SHELPER_H
#include "interp.hpp"
#include <cstdint>

#define LAMBDA_FROM_EXPR(e) [](){return e;}

void setHelperInterpreter(Interpreter *itr);
const char* valueof(string input_id);
bool defined(string input_id);
void output(string output_id, string val);
uint32_t tokens(string place_id);
uint64_t elapsed(string place_or_transition_id);
uint64_t now();
#endif