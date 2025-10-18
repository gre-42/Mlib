#include "Objective.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

Objective Mlib::objective_from_string(const std::string& s) {
    static const std::map<std::string, Objective> m{
        {"kill_count", Objective::KILL_COUNT},
        {"last_team_standing", Objective::LAST_TEAM_STANDING},
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown objective: \"" + s + '"');
    }
    return it->second;
}
