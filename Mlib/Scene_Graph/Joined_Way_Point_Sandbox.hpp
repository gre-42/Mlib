#pragma once
#include <string>

namespace Mlib {

enum class JoinedWayPointSandbox {
    NONE = 0,
    STREET = 1 << 0,
    SIDEWALK = 1 << 1,
    EXPLICIT_GROUND = 1 << 2,
    RUNWAY_OR_TAXIWAY_OR_AIRWAY = 1 << 3
};

static inline bool any(JoinedWayPointSandbox j) {
    return j != JoinedWayPointSandbox::NONE;
}

static inline JoinedWayPointSandbox& operator |= (JoinedWayPointSandbox& a, JoinedWayPointSandbox b) {
    return (JoinedWayPointSandbox&)((int&)a |= (int)b);
}

static inline JoinedWayPointSandbox operator | (JoinedWayPointSandbox a, JoinedWayPointSandbox b) {
    return (JoinedWayPointSandbox)((int)a | (int)b);
}

static inline JoinedWayPointSandbox operator & (JoinedWayPointSandbox a, JoinedWayPointSandbox b) {
    return (JoinedWayPointSandbox)((int)a & (int)b);
}

std::string joined_way_point_sandbox_to_string(JoinedWayPointSandbox t);
JoinedWayPointSandbox joined_way_point_sandbox_from_string(const std::string& s);

}
