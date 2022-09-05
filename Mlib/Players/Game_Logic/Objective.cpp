#include "Objective.hpp"
#include <stdexcept>

using namespace Mlib;

Objective Mlib::objective_from_string(const std::string& s) {
    if (s == "last_team_standing") {
        return Objective::LAST_TEAM_STANDING;
    } else if (s == "kill_count") {
        return Objective::KILL_COUNT;
    } else {
        throw std::runtime_error("Unknown objective: \"" + s + '"');
    }
}
