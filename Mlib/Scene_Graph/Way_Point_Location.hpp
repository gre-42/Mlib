#pragma once
#include <string>

namespace Mlib {

enum class WayPointLocation {
    UNKNOWN,
    STREET,
    SIDEWALK,
    EXPLICIT
};

std::string way_point_location_to_string(WayPointLocation wpl);

}
