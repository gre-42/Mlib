#pragma once
#include <string>

namespace Mlib {

struct Way;

enum class WayPointsOrientation {
    UNIDIRECTIONAL,
    BIDIRECTIONAL
};

WayPointsOrientation way_point_orientation_from_string(const std::string& orientation);

struct TerrainWayPoints {
    const Way& way;
    WayPointsOrientation orientation;
};

}
