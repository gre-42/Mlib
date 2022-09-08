#include "Way_Point_Location.hpp"
#include <stdexcept>

using namespace Mlib;

std::string Mlib::way_point_location_to_string(WayPointLocation wpl) {
    if (wpl == WayPointLocation::UNKNOWN) {
        return "unknown";
    } else if (wpl == WayPointLocation::STREET) {
        return "street";
    } else if (wpl == WayPointLocation::SIDEWALK) {
        return "sidewalk";
    } else if (wpl == WayPointLocation::EXPLICIT) {
        return "explicit";
    } else {
        throw std::runtime_error("Unknown waypoint location");
    }
}
