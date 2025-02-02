#pragma once
#include <Mlib/Osm_Loader/Osm_Map_Resource/Height.hpp>
#include <string>

namespace Mlib {

struct Way;

enum class WayPointsOrientation {
    UNIDIRECTIONAL,
    BIDIRECTIONAL
};

WayPointsOrientation way_points_orientation_from_string(const std::string& orientation);

enum class WayPointsClass {
    NONE = 0,
    GROUND = 1 << 0,
    AIRWAY = 1 << 1
};

WayPointsClass way_points_class_from_string(const std::string& class_);

static inline WayPointsClass operator | (WayPointsClass a, WayPointsClass b) {
    return (WayPointsClass)((int)a | (int)b);
}

static inline WayPointsClass operator & (WayPointsClass a, WayPointsClass b) {
    return (WayPointsClass)((int)a & (int)b);
}

static inline bool any(WayPointsClass a) {
    return a != WayPointsClass::NONE;
}

// Note that this struct is way-based, not node-based.
struct TerrainWayPoints {
    const Way& way;
    WayPointsOrientation orientation;
    WayPointsClass class_;
};

}
