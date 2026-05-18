/**
 * @file scripting_helper.hpp
 * @author Ondřej Turek, xtureko00
 * @brief Inscription language helper functions interface
 */
#ifndef _SHELPER_H
#define _SHELPER_H
#include "interp.hpp"
#include <cstdint>

#define LAMBDA_FROM_EXPR(e) [&](){return e;}
#define LAMBDA_FROM_EXPR_NORETURN(e) [&](){e}

void setHelperInterpreter(Interpreter *itr);
const char* valueof(string input_id);
bool defined(string input_id);
void output(string output_id, string val);
uint32_t tokens(string place_id);
uint64_t elapsed(string place_or_transition_id);
uint64_t now();
#endif