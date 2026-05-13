#ifndef _GENINTERP_H
#define _GENINTERP_H

#include "editorstate.hpp"
#include <filesystem>
#include <sstream>
#include <string>
#include <QString>

const std::string MAIN_FILENAME = "program.cpp";
const std::string MAIN_GENERATED = "program.generated.cpp";

class InterpreterGenerator {
    public:
        void setPath(const std::filesystem::path path);
        std::filesystem::path getPath();
        void setEntrypoint(const std::string filename);
        void setMarker(const std::string);
        bool generateMain(const PetriNetworkSpec *spec);
        bool compile();
        bool run();
    
    private:
        void emitPlace(const PetriPlace *p);
        void emitTransition(const PetriTransition *t);
        void emitArc(const PetriArc *a);
        void emitAll(const PetriNetworkSpec *spec);

        std::filesystem::path interpSourcePath;
        std::string interpEntryFilename;
        QString interpDefMarker;
        std::ostringstream generatedBuffer;
};

#endif