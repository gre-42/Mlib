#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class ResourceUpdateCycle {
    ONCE,
    ALWAYS
};

inline ResourceUpdateCycle resource_update_cycle_from_string(const std::string& str) {
    if (str == "once") {
        return ResourceUpdateCycle::ONCE;
    } else if (str == "always") {
        return ResourceUpdateCycle::ALWAYS;
    }
    throw std::runtime_error("Unknown render to texture update cycle: " + str);
}

}
