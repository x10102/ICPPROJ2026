// program.cpp - program entry point
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <memory.h>
#include "interp.hpp"
#include "petri.hpp"
#include "debug.hpp"
#include "scripting_helper.hpp"
#include "gui/picojson.h"

#define NETWORK_BUFFER_SIZE 8192

#define PLACE(var, id, tok) Place *var = interp.createPlace(id, tok)
#define TRANSITION(var, id) Transition *var = interp.createTransition(id)
#define CONDITION(tr, name, delay) tr->setFireCondition({delay, name})
#define CONDITION_EXPR(tr, name, delay, expr) tr->setFireCondition({delay, name, LAMBDA_FROM_EXPR(expr)})
#define ACTION(tr, expr) tr->setAction(LAMBDA_FROM_EXPR(expr));
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
    ACTION(t2, output("amongusovni_vystup", to_string(now())))

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
    
    Interpreter interp;
    unsigned int port;
    port = 6768;
    
    setHelperInterpreter(&interp);
    /*
    PLACE(a1, "prvni" ,1);
    PLACE(a2, "druhy", 0);
    PLACE(a3, "treti", 100);
    PLACE(a4, "ctvrty", 0);
    
    TRANSITION(t2, "start_time_cond");
    ENTRY_EDGE(a1, t2, 1);
    EXIT_EDGE(t2, a2, 67);
    CONDITION_EXPR(t2, "", 0, now() >= 5000);
    ACTION(t2, output("amongusovni_vystup", to_string(now())))

    TRANSITION(t3, "input_event_cond");
    ENTRY_EDGE(a3, t3, 100);
    EXIT_EDGE(t3, a4, 5);
    CONDITION(t3, "sestsedm", 0);
    ACTION(t3, output("sixsevenovni vystup", "ahojahojahoj"));
    */

    // #### MARKER ####

    // TODO: Move allll of this shit to the interpreter class, maybe make a separate network handler class to not clutter the petri logic too much
    struct sockaddr_in editor_addr;
    struct sockaddr_in self_addr;
    char netbuffer[NETWORK_BUFFER_SIZE];
    memset(&editor_addr, 0, sizeof(struct sockaddr_in));
    memset(&self_addr, 0, sizeof(struct sockaddr_in));
    self_addr.sin_addr.s_addr = INADDR_ANY;
    self_addr.sin_family = AF_INET;
    self_addr.sin_port = htons(port);
    int sock_recv = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_recv < 0) {
        LOG_I("Application error: failed to create socket");
        exit(2);
    }
    socklen_t saddr = sizeof(struct sockaddr_in);
    if(bind(sock_recv, (struct sockaddr*)&self_addr, saddr) < 0) {
        LOG_I("Application error: failed to bind socket");
        exit(2);
    }

    int recv_len;
    
    while(true) {
        picojson::value data;
        recv_len = recvfrom(sock_recv, netbuffer, NETWORK_BUFFER_SIZE, MSG_WAITALL, (struct sockaddr*)&editor_addr, &saddr);
        editor_addr.sin_port = htons(port-1);
        std::string decode_error = picojson::parse(data, netbuffer);
        if(!decode_error.empty() || !data.is<picojson::object>()) {
            LOG_I("Application error: received malformed command, ignoring it.");
            continue;
        }
        auto payload = data.get<picojson::object>();
        std::string command = payload["command"].to_str();
        if(command.compare("step") == 0) {
            // With the way the interpreter is programmed, we can really only do one step at a time
            // We could add a parameter for N steps
            // And also a parameter for "half-steps" - the doTransitions() loop will only run for one cycle and then report back state
            // Would make it easier to follow convoluted networks where lots of transitions will fire each cycle
            interp.doTransitions();
        } else if(command.compare("event") == 0) {
            interp.inputEvent(payload["eventName"].to_str(), payload["eventVal"].to_str());
        } else if(command.compare("exit") == 0) {
            interp.terminate();
            break;
        }
        std::string json = picojson::value(interp.json()).serialize(false);
        json.copy(netbuffer, NETWORK_BUFFER_SIZE);
        // Terminate the buffer manually because string.copy() doesn't do that apparently???
        netbuffer[json.length()] = '\0';
        sendto(sock_recv, netbuffer, strlen(netbuffer), 0, (struct sockaddr*)&editor_addr, saddr);
        // Clear the events that fired
        interp.clearFired();
        interp.clearEvents();
    }

    //interactive_test();
    return 0;
}