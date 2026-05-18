/**
 * @file program.cpp
 * @author Ondřej Turek, xtureko00
 * @brief Interpreter entry point
 */
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <memory.h>
#include "interp.hpp"
#include "debug.hpp"
#include "scripting_helper.hpp"

#define PLACE(var, id, tok) Place *var = __INTERNAL_interp.createPlace(id, tok)
#define TRANSITION(var, id) Transition *var = __INTERNAL_interp.createTransition(id)
#define CONDITION(tr, name, delay) tr->setFireCondition({delay, name})
#define CONDITION_EXPR(tr, name, delay, expr) tr->setFireCondition({delay, name, LAMBDA_FROM_EXPR(expr)})
#define ACTION(tr, expr) tr->setAction(LAMBDA_FROM_EXPR_NORETURN(expr));
#define ENTRY_EDGE(from, tr, weight) tr->addEntryEdge(from, weight)
#define EXIT_EDGE(tr, to, weight) tr->addExitEdge(to, weight)

using namespace std;

void __INTERNAL_interactive_test(Interpreter *interp) {

    LOG_I("Enter interactive test");
    LOG_I("Trigger events with: <event> <value>");
    LOG_I("Enter \"(r)un_net\" to connect GUI");
    LOG_I("Enter \"(s)tep\" to do single iteration of transitions");
    LOG_I("Enter \"(c)ontinue\" to do all possible transitions");
    LOG_I("Enter blank line to print state");
    LOG_I("Enter \"(e)xit\" to end");
    while(true) {
        string buffer;
        cout << "interp# ";
        getline(cin, buffer);
        size_t space_pos = buffer.find(" ");

        if(space_pos == string::npos) {
            if(buffer.compare("exit") == 0 || buffer[0] == 'e') {
                interp->terminate();
                break;
            } else if(buffer.compare("step") == 0 || buffer[0] == 's') {
                const uint step_count = std::count(buffer.begin(), buffer.end(), 's');
                for(uint i = 0; i < step_count; i++)
                    interp->doTransitions(false);
                interp->printState();
            } else if(buffer.compare("continue") == 0 || buffer[0] == 'c') {
                interp->doTransitions();
                interp->printState();
            } else if(buffer.compare("run_net") == 0 || buffer[0] == 'r') {
                LOG_I("Running on default port 6768\nPress CTRL+C or send terminate command to exit.");
                interp->run(6768);
                break;
            } else if(buffer.compare("\n")) {
                interp->printState();
            } else {
                cout << "INVALID" << endl;
            }
            continue;
        }
        string evt_name = buffer.substr(0, space_pos);
        string evt_val = buffer.substr(space_pos+1, buffer.length()-space_pos);
        interp->inputEvent(evt_name, evt_val);
    }
}

// Prefix any local variables here with __INTERNAL_
// To prevent clash with net variables (user-defined)
int main(int argc, char *argv[]) {
    
    Interpreter __INTERNAL_interp;
    unsigned int __INTERNAL_port;
    __INTERNAL_port = 6768;
    
    setHelperInterpreter(&__INTERNAL_interp);

    // #### MARKER ####

    if(argc >= 2 && strcmp(argv[1], "--interactive") == 0) {
        __INTERNAL_interactive_test(&__INTERNAL_interp);
    } else {
        __INTERNAL_interp.run(__INTERNAL_port);
    }
    return 0;
}