#include "Resource_Update_Cycle.hpp"
#include <stdexcept>

using namespace Mlib;

ResourceUpdateCycle Mlib::resource_update_cycle_from_string(const std::string& str) {
    if (str == "once") {
        return ResourceUpdateCycle::ONCE;
    } else if (str == "always") {
        return ResourceUpdateCycle::ALWAYS;
    }
    throw std::runtime_error("Unknown render to texture update cycle: " + str);
}
