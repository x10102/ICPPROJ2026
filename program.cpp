// program.cpp - program entry point
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>
#include <iostream>
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

    interp.inputEvent("hi", "hi");
    interp.inputEvent("lmao", "lol");
    interp.waitForAllTimers();
    interp.printState();
}

int main(int argc, char *argv[]) {
    LOG_I("Enter interactive test")
    interactive_test();
    return 0;
}