#pragma once
#include <string>

namespace Mlib {

enum class WayPointLocation {
    NONE = 0,
    UNKNOWN = 1 << 0,
    STREET = 1 << 1,
    SIDEWALK = 1 << 2,
    RUNWAY_OR_TAXIWAY = 1 << 3,
    EXPLICIT_GROUND = 1 << 4,
    RUNWAY_OR_TAXIWAY_OR_AIRWAY = 1 << 5
};

static inline bool any(WayPointLocation a) {
    return a != WayPointLocation::NONE;
}

static inline WayPointLocation operator |= (WayPointLocation& a, WayPointLocation b) {
    (int&)a |= (int)b;
    return a;
}

static inline WayPointLocation operator | (WayPointLocation a, WayPointLocation b) {
    return (WayPointLocation)((int)a | (int)b);
}

static inline WayPointLocation operator & (WayPointLocation a, WayPointLocation b) {
    return (WayPointLocation)((int)a & (int)b);
}

std::string way_point_location_to_string(WayPointLocation t);
WayPointLocation way_point_location_from_string(const std::string& s);

}
