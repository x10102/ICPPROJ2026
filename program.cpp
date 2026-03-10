// program.cpp - program entry point
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>
#include <cstddef>
#include <iostream>
#include <string>
#include "interp.hpp"
#include "petri.hpp"
#include "debug.hpp"

using namespace std;

void interactive_test() {
    Interpreter interp;

    Place *a1 = interp.createPlace("amogus misto", 1);
    Place *a2 = interp.createPlace("diddy misto", 0);
    Place *a3 = interp.createPlace("secret misto", 0);
    Place *a4 = interp.createPlace("izolovane misto", 4);
    Place *a5 = interp.createPlace("druhe tajne misto", 0);
    Transition *t1 = interp.createTransition("autobus");
    Transition *t2 = interp.createTransition("metro");
    Transition *t3 = interp.createTransition("nejaka vec");

    t1->addEntryEdge(a1, 1);
    t1->addExitEdge(a2, 1);

    t2->addEntryEdge(a1, 1);
    t2->addExitEdge(a3, 1);

    t3->addEntryEdge(a4, 4);
    t3->addExitEdge(a5, 4);
    t3->setFireCondition({1000, "lmao"});

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
        string evt_val = buffer.substr(space_pos, buffer.length()-space_pos);
        interp.inputEvent(evt_name, evt_val);
    }
}

int main(int argc, char *argv[]) {
    
    interactive_test();
    return 0;
}