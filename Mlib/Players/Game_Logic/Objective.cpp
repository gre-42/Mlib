#include "Objective.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

Objective Mlib::objective_from_string(const std::string& s) {
    if (s == "last_team_standing") {
        return Objective::LAST_TEAM_STANDING;
    } else if (s == "kill_count") {
        return Objective::KILL_COUNT;
    } else {
        THROW_OR_ABORT("Unknown objective: \"" + s + '"');
    }
}
