// program.cpp - program entry point
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>
#include "interp.hpp"
#include "petri.hpp"
#include "debug.hpp"

using namespace std;

int main(int argc, char *argv[]) {

    Place a1("amogus misto", 1);
    Place a2("diddy misto", 0);
    Place a3("secret misto", 0);
    Place a4("izolovane misto", 4);
    Place a5("druhe tajne misto", 0);
    Transition t1("autobus");
    Transition t2("metro");
    Transition t3("nejaka vec");

    t1.addEntryEdge(&a1, 1);
    t1.addExitEdge(&a2, 1);

    t2.addEntryEdge(&a1, 1);
    t2.addExitEdge(&a3, 1);

    t3.addEntryEdge(&a4, 4);
    t3.addExitEdge(&a5, 4);
    t3.setFireCondition({1000, "lmao"});

    auto a = {a1, a2};

    Interpreter interp;
    interp.addPlaces({a1, a2, a3, a4, a5});
    interp.addTransitions({t1, t2, t3});
    interp.inputEvent("hi", "hi");
    interp.inputEvent("lmao", "lol");
    interp.waitForAllTimers();
    return 0;
}