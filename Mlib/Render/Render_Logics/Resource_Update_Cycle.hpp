#pragma once
#include <string>

namespace Mlib {

enum class ResourceUpdateCycle {
    ONCE,
    ALWAYS
};

ResourceUpdateCycle resource_update_cycle_from_string(const std::string& str);

}
