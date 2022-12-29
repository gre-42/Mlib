#include "Terrain_Way_Points.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

WayPointsOrientation Mlib::way_point_orientation_from_string(const std::string& orientation) {
    if (orientation == "unidirectional") {
        return WayPointsOrientation::UNIDIRECTIONAL;
    } else if (orientation == "bidirectional") {
        return WayPointsOrientation::BIDIRECTIONAL;
    } else {
        THROW_OR_ABORT("Unknown waypoints orientation: \"" + orientation + '"');
    }
}
