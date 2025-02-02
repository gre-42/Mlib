#pragma once
#include <string>

namespace Mlib {

enum class SmoothnessTarget {
    PHYSICS = (1 << 0),
    RENDER = (1 << 1)
};

SmoothnessTarget smoothness_target_from_string(const std::string& s);

}
