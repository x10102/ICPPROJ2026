#include "geninterp.hpp"
#include "editorstate.hpp"
#include <filesystem>
#include <ostream>
#include <qchar.h>
#include <qevent.h>
#include <qlist.h>
#include <qlocale.h>
#include <QTextStream>
#include <qregion.h>
#include <qstringliteral.h>

using namespace std;

filesystem::path InterpreterGenerator::getPath() {
    return this->interpSourcePath;
}

void InterpreterGenerator::emitPlace(const PetriPlace *p) {
    this->generatedBuffer << "PLACE(";
    this->generatedBuffer << p->name << ", \"" << p->name << "\", ";
    this->generatedBuffer << p->initial_tokens;
    this->generatedBuffer << ");" << std::endl;
    if(!p->placeActionMacro.empty()) {
        this->generatedBuffer << "ACTION(";
        this->generatedBuffer << p->name << ", ";
        this->generatedBuffer << p->placeActionMacro;
        this->generatedBuffer << ")" << std::endl;
    }
}

void InterpreterGenerator::emitTransition(const PetriTransition *t) {
    generatedBuffer << "TRANSITION(";
    generatedBuffer << t->name << ", \"" << t->name << "\");" << std::endl;
    if((t->delayMs != 0 || !t->inputEventName.empty())) {
        if(!t->booleanGuardMacro.empty()) {
            generatedBuffer << "CONDITION_EXPR(" << t->name;
            generatedBuffer << "\"" << t->inputEventName << "\", ";
            generatedBuffer << t->delayMs << ", ";
            generatedBuffer << t->booleanGuardMacro << ");" << std::endl;
        } else {
            generatedBuffer << "CONDITION(" << t->name;
            generatedBuffer << "\"" << t->inputEventName << "\", ";
            generatedBuffer << t->delayMs << ");" << std::endl;
        }
    }
    if(!t->tranActionMacro.empty()) {
        generatedBuffer << "ACTION(" << t->name << ", " << t->tranActionMacro << ");" << std::endl;
    }
    
}

void InterpreterGenerator::emitArc(const PetriArc *a) {
    if(a->type == PLACE_TO_TRANSITION) {
        generatedBuffer << "ENTRY_EDGE(" << a->place->name << ", ";
        generatedBuffer << a->transition->name << ", ";
        generatedBuffer << a->tokenCount << ");" << std::endl;
    } else if(a->type == TRANSITION_TO_PLACE) {
        generatedBuffer << "EXIT_EDGE(" << a->transition->name << ", ";
        generatedBuffer << a->place->name << ", ";
        generatedBuffer << a->tokenCount << ");" << std::endl;
    }
}

void InterpreterGenerator::emitAll(const PetriNetworkSpec *spec) {
    for(const auto &p : spec->places)
        this->emitPlace(&p.second);
    for(const auto &tr : spec->transitions)
        this->emitTransition(&tr.second);
    for(const auto &arc : spec->arcs)
        this->emitArc(&arc.second); 
}

// TODO: Make this a signal?
bool InterpreterGenerator::generateMain(const PetriNetworkSpec *spec) {
    this->generatedBuffer.clear();
    this->emitAll(spec);
    // TODO: Raise an exception
    filesystem::path mainPath = this->interpSourcePath / MAIN_FILENAME;
    if(!filesystem::exists(mainPath)) {
        cerr << "Error generating interpreter: file " << mainPath << " doesn't exist" << endl;
        return false;
    }
    QFile source(QString::fromStdString(mainPath));
    if(!source.open(QIODevice::ReadOnly | QIODevice::Text)) {
        cerr << "Error generating interpreter: file " << mainPath << " cannot be opened" << endl;
        return false;
    }
    QTextStream inputStream(&source);
    QStringList outSourceLines;

    while(!inputStream.atEnd()) {
        QString currentLine = inputStream.readLine();
        if(currentLine.contains(interpDefMarker)) {
            currentLine.replace("// " + interpDefMarker, QString::fromStdString(generatedBuffer.str()));
        }
        outSourceLines.append(currentLine);
    }
    source.close();

    filesystem::path outputPath = this->interpSourcePath / MAIN_GENERATED;
    QFile output(QString::fromStdString(outputPath));

    if(!output.open(QIODevice::WriteOnly | QIODevice::Text)) {
        cerr << "Could not write output source file" << endl;
        return false;
    }

    QTextStream outputStream(&output);
    for(const auto &line : outSourceLines) {
        outputStream << line << Qt::endl;
    }
    return true;
}

void InterpreterGenerator::setMarker(std::string marker) {
    this->interpDefMarker = QString::fromStdString(marker);
}

void InterpreterGenerator::setPath(std::filesystem::path path) {
    this->interpSourcePath = path;
}