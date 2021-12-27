#include "Way_Points.hpp"
#include <stdexcept>

using namespace Mlib;

WayPointsOrientation Mlib::way_point_orientation_from_string(const std::string& orientation) {
    if (orientation == "unidirectional") {
        return WayPointsOrientation::UNIDIRECTIONAL;
    } else if (orientation == "bidirectional") {
        return WayPointsOrientation::BIDIRECTIONAL;
    } else {
        throw std::runtime_error("Unknown waypoints orientation: \"" + orientation + '"');
    }
}
