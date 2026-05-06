#ifndef _GENINTERP_H
#define _GENINTERP_H

#include "editorstate.hpp"
#include <filesystem>
#include <sstream>
#include <string>
class InterpreterGenerator {
    public:
        void setPath(const std::filesystem::path path);
        void setEntrypoint(const std::string filename);
        void setMarker(const std::string);
        bool generateMain(const PetriNetworkSpec *spec);
        bool compile();
        bool run();
    
    private:
        void emitPlace(const PetriPlace *p);
        void emitTransition(const PetriTransition *t);
        void emitArc(const PetriArc *a);

        std::filesystem::path interpSourcePath;
        std::string interpEntryFilename;
        std::string interpDefMarker;
        std::ostringstream generatedBuffer;
};

#endif