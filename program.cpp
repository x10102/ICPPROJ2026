#include <iostream>

#include "interp.hpp"
#include "petri.hpp"
#include "debug.hpp"

using namespace std;

int main(int argc, char *argv[]) {

    Place a1("amogus misto", 1);
    Place a2("diddy misto", 0);
    Place a3("secret misto", 0);
    Transition t1("autobus");
    Transition t2("metro");
    t1.addEntryEdge(&a1, 1);
    t1.addExitEdge(&a2, 1);
    t2.addEntryEdge(&a1, 1);
    t2.addExitEdge(&a3, 1);
    Interpreter interp;
    interp.add_place(a1);
    interp.add_place(a2);
    interp.add_transition(t1);
    interp.add_transition(t2);
    interp.do_immediate_transitions();
    return 0;
}