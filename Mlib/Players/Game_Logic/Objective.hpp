#pragma once
#include <string>

namespace Mlib {

enum class Objective {
    NONE,
    KILL_COUNT,
    LAST_TEAM_STANDING,
    PHOTOS,
};

Objective objective_from_string(const std::string& s);

}
