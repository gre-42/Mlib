#include "Objective.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

Objective Mlib::objective_from_string(const std::string& s) {
    static const std::map<std::string, Objective> m{
        {"kill_count", Objective::KILL_COUNT},
        {"last_team_standing", Objective::LAST_TEAM_STANDING},
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown objective: \"" + s + '"');
    }
    return it->second;
}
