#include "Race_Identifier.hpp"
#include <sstream>

using namespace Mlib;

void check_dirname_part(const std::string& part) {
    for (char c : part) {
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
        throw std::runtime_error("Invalid charactar in dirname part (must be 0-9, A-Z, a-z, _ or -)");
    }
}

std::string RaceIdentifier::dirname() const {
    if (level.empty()) {
        throw std::runtime_error("Empty level name");
    }
    if (session.empty()) {
        throw std::runtime_error("Empty session name");
    }
    check_dirname_part(level);
    check_dirname_part(session);
    std::stringstream sstr;
    sstr << level << '.' << session;
    if (laps != 0) {
        sstr << "." << laps;
    }
    if (milliseconds != 0) {
        sstr << "." << milliseconds;
    }
    return sstr.str();
}