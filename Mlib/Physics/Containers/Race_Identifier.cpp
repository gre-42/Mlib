#include "Race_Identifier.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
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
        if ((c == '_') || (c == '-') || (c == '.')) {
            continue;
        }
        THROW_OR_ABORT("Invalid charactar in dirname part (must be 0-9, A-Z, a-z, _ or -): \"" + part + '"');
    }
}

std::string RaceIdentifier::dirname() const {
    if (level.empty()) {
        THROW_OR_ABORT("Empty level name");
    }
    if (session.empty()) {
        THROW_OR_ABORT("Empty session name");
    }
    check_dirname_part(level);
    check_dirname_part(time_of_day);
    check_dirname_part(restrictions);
    check_dirname_part(session);
    std::stringstream sstr;
    sstr << level << '.' << session;
    if (time_of_day != "day") {
        sstr << '.' << time_of_day;
    }
    if (!restrictions.empty()) {
        sstr << '.' << restrictions;
    }
    if (laps != 0) {
        sstr << "." << laps;
    }
    if (milliseconds != 0) {
        sstr << "." << milliseconds;
    }
    return sstr.str();
}
