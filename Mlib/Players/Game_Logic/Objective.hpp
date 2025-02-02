#pragma once
#include <string>

namespace Mlib {

enum class Objective {
    NONE,
    LAST_TEAM_STANDING,
    KILL_COUNT
};

Objective objective_from_string(const std::string& s);

}
