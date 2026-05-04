// program.cpp - program entry point
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>
#include <cstddef>
#include <iostream>
#include <string>
#include "interp.hpp"
#include "petri.hpp"
#include "debug.hpp"
#include "scripting_helper.hpp"

#define PLACE(var, id, tok) Place *var = interp.createPlace(id, tok)
#define TRANSITION(var, id) Transition *var = interp.createTransition(id)
#define CONDITION(tr, name, delay) tr->setFireCondition({delay, name})
#define CONDITION_EXPR(tr, name, delay, expr) tr->setFireCondition({delay, name, LAMBDA_FROM_EXPR(expr)})
#define TRANSITION_ACTION(tr, expr) tr->setAction(LAMBDA_FROM_EXPR(expr));
#define ENTRY_EDGE(from, tr, weight) tr->addEntryEdge(from, weight)
#define EXIT_EDGE(tr, to, weight) tr->addExitEdge(to, weight)

using namespace std;

void interactive_test() {
    Interpreter interp;

    PLACE(a1, "prvni" ,1);
    PLACE(a2, "druhy", 0);
    TRANSITION(t2, "start_time_cond");
    ENTRY_EDGE(a1, t2, 1);
    EXIT_EDGE(t2, a2, 67);
    CONDITION_EXPR(t2, "", 0, now() >= 5000);
    TRANSITION_ACTION(t2, output("amongusovni_vystup", to_string(now())))

    LOG_I("Enter interactive test");
    LOG_I("Trigger events with: <event> <value>")
    LOG_I("Enter blank line to print state")
    LOG_I("Enter \"exit\" to end")
    while(true) {
        string buffer;
        cout << "interp# ";
        getline(cin, buffer);
        size_t space_pos = buffer.find(" ");

        if(space_pos == string::npos) {
            if(buffer.compare("exit") == 0) {
                interp.terminate();
                break;
            } else if(buffer.compare("\n")) {
                interp.printState();
            } else {
                cout << "INVALID" << endl;
            }
            continue;
        }
        string evt_name = buffer.substr(0, space_pos);
        string evt_val = buffer.substr(space_pos+1, buffer.length()-space_pos);
        interp.inputEvent(evt_name, evt_val);
    }
}

int main(int argc, char *argv[]) {
    
    interactive_test();
    return 0;
}