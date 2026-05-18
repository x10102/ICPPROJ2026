/**
 * @file scripting_helper.hpp
 * @author Ondřej Turek, xtureko00
 * @brief Inscription language helper functions
 */
#include "scripting_helper.hpp"
#include "interp.hpp"
#include "debug.hpp"
#include <chrono>

using namespace chrono;

// One evil global variable per app is okay, right? >w<
// (We could like make a HelperManager class with this as instance member,
// but then we'd have to use a lot of evil macros instead)
Interpreter *interpreter;

// This could be an interpreter instance member, but I don't know yet if the interpreter
// will always be initialized at/microseconds after app start
static const milliseconds START_TIME = duration_cast<milliseconds>(steady_clock::now().time_since_epoch());

// This should be called only once
// Sets the interpreter that the helper functions will refer to
void setHelperInterpreter(Interpreter *itr) {
    interpreter = itr;
}

// Helper function - returns the current millisecond count from the steady_clock
inline static milliseconds current_ms(void) {
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch());
}

// Returns the last value of the input with the specified id
const char* valueof(string input_id) {
    if(!interpreter->inputDefined(input_id))
        return nullptr;
    const string* last_val = interpreter->lastInputValue(input_id);
    // This should be fine - c_str returns a pointer to the string object's internal buffer
    // That string object is owned by the interpreter instance, which will in any case outlive expressions using this
    return last_val->c_str();
}

// Returns true if the input was set at any point
bool defined(string input_id) {
    return interpreter->inputDefined(input_id);
}

// Generates an output event
void output(string output_id, string val) {
    interpreter->outputEvent(output_id, val);
}

// Returns the current number of tokens in the specified place
uint32_t tokens(string place_id) {
    Place *place = interpreter->getPlace(place_id);
    if(place == nullptr) return 0;
    return place->getTokenCount();
}

// Returns the time elapsed since the number of tokens in the
// specified place last changed
// As for the transition, idk bro
uint64_t elapsed(string place_or_transition_id) {
    Place *p = interpreter->getPlace(place_or_transition_id);
    if(p == nullptr) {
        // TODO: Idk what we're supposed to do here, project spec doesn't make it too clear
        // Have to ask
        LOG_I("elapsed() called for transition or nonexistent place - not implemented");
        return 0;
    }
    return (current_ms() - p->getLastChangeTime()).count();
}

// Returns the number of milliseconds elapsed since the program started
uint64_t now() {
    return (current_ms() - START_TIME).count();
}   