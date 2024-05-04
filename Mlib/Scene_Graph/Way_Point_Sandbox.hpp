#pragma once
#include <string>

namespace Mlib {

enum class WayPointSandbox {
    STREET,
    SIDEWALK,
    EXPLICIT_GROUND,
    RUNWAY_OR_TAXIWAY
};

std::string way_point_sandbox_to_string(WayPointSandbox t);
WayPointSandbox way_point_sandbox_from_string(const std::string& s);

}
