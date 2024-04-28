#include "Terrain_Way_Points.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

WayPointsOrientation Mlib::way_points_orientation_from_string(const std::string& orientation) {
    static const std::map<std::string, WayPointsOrientation> m{
        { "unidirectional", WayPointsOrientation::UNIDIRECTIONAL },
        { "bidirectional", WayPointsOrientation::BIDIRECTIONAL }
    };
    auto it = m.find(orientation);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown waypoints orientation: \"" + orientation + '"');
    }
    return it->second;
}

WayPointsClass Mlib::way_points_class_from_string(const std::string& class_) {
    static const std::map<std::string, WayPointsClass> m{
        { "ground", WayPointsClass::GROUND },
        { "airway", WayPointsClass::AIRWAY }
    };
    auto it = m.find(class_);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown waypoints class: \"" + class_ + '"');
    }
    return it->second;
}
