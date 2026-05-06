#ifndef _GENINTERP_H
#define _GENINTERP_H

#include "editorstate.hpp"
#include <filesystem>
#include <sstream>
#include <string>
class InterpreterGenerator {
    public:
        void setPath(std::filesystem::path path);
        void setEntrypoint(std::string filename);
        void setMarker(std::string);
        bool compile(PetriNetworkSpec spec);
        bool run();
    
    private:
        void emitPlace(PetriPlace *p);
        void emitTransition(PetriTransition *t);
        void emitArc(PetriArc *a);

        std::filesystem::path interpSourcePath;
        std::string interpEntryFilename;
        std::string interpDefMarker;
        std::ostringstream generatedBuffer;
};

#endif