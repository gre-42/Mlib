#include "Race_Configuration.hpp"
#include <sstream>

using namespace Mlib;

std::string RaceConfiguration::dirname() const {
    for (char c : session) {
        if ((c >= '0') && (c <= '9')) {
            continue;
        }
        if ((c >= 'A') && (c <= 'Z')) {
            continue;
        }
        if ((c >= 'a') && (c <= 'z')) {
            continue;
        }
        if ((c == '_') || (c == '-')) {
            continue;
        }
        throw std::runtime_error("Invalid charactar in session name (must be 0-9, A-Z, a-z, _ or -)");
    }
    std::stringstream sstr;
    sstr << session;
    if (laps != 0) {
        sstr << "." << laps;
    }
    if (milliseconds != 0) {
        sstr << "." << milliseconds;
    }
    return sstr.str();
}
