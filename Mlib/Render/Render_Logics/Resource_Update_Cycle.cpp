#include "Resource_Update_Cycle.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ResourceUpdateCycle Mlib::resource_update_cycle_from_string(const std::string& str) {
    if (str == "once") {
        return ResourceUpdateCycle::ONCE;
    } else if (str == "always") {
        return ResourceUpdateCycle::ALWAYS;
    }
    THROW_OR_ABORT("Unknown render to texture update cycle: " + str);
}
