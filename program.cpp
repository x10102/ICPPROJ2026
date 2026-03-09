#include <iostream>

#include "petri.hpp"
#include "debug.hpp"

using namespace std;

int main(int argc, char *argv[]) {

    Place a1("amogus misto", 1);
    Place a2("diddy misto", 0);
    Transition t1("autobus");
    t1.addEntryEdge(&a1, 1);
    t1.addExitEdge(&a2, 1);
    t1.fire();
    return 0;
}