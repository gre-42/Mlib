#include "Way_Point_Location.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

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
        THROW_OR_ABORT("Unknown waypoint location");
    }
}
