// interp.hpp - interpreter interface
// Authors:
// - Ondřej Turek <xtureko00@stud.fit.vutbr.cz>
// - Adam Šrámek <xsramea00@stud.fit.vutbr.cz>

#ifndef _INTERP_H
#define _INTERP_H

#include "petri.hpp"
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <mutex>

struct Variable {
    string type;
    string name;
    string value;
};

using namespace std;
class Interpreter {

    protected:
        //Metadata pro save a load
        string netName;
        string netComment;
        unordered_map<string, string> inputValues;
        unordered_map<string, string> outputValues;
        vector<Variable> variables;
        unordered_map<string, unique_ptr<Place>> places;
        unordered_map<string, unique_ptr<Transition>> transitions;


        unordered_map<string, uint32_t> transitionOrder;
        vector<thread> timerThreads;
        string last_input;
        mutex transition_lock;
        uint32_t max_order;
        bool exiting;

        void delayedFire(Transition *tr, uint32_t delay_ms);

        void parseLoadFileLine(const string &section, const string &line);
    public:
        bool save(const string &filename);
        bool load(const string &filename);
        void clearNet();

        Place* createPlace(string identifier, uint32_t initial_tokens);
        Place* getPlace(string identifier);
        bool renamePlace(const string &oldId, const string &newId);
        bool removePlace(const string &identifier);
        Transition* createTransition(string identifier);
        bool renameTransition(const string &oldId, const string &newId);
        bool removeTransition(const string &identifier);
        void doTransitions();
        void inputEvent(const std::string input, const std::string value);
        virtual void outputEvent(const std::string output, const std::string value);
        void waitForAllTimers();
        void terminate();
        virtual void printState();
        bool inputDefined(const std::string input);
        std::string *lastInputValue(const std::string input);
        Interpreter();

};

#endif