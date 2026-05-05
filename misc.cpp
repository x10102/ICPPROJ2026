// misc.cpp - miscellaneous functions
// Authors:
// - Adam Šrámek <xsramea00@stud.fit.vutbr.cz>


#include "misc.hpp"
#include <string>

std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    
    if (first == std::string::npos) {
        return "";
    }

    size_t last = s.find_last_not_of(" \t\n\r");

    return s.substr(first, (last - first + 1));
}