#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Type.hpp>
#include <string>

namespace Mlib {

enum class WayPointLocation;

struct StreetRectangle {
    WayPointLocation location;
    RoadProperties road_properties;
    std::string bumps_model;
    FixedArray<FixedArray<double, 3>, 2, 2> rectangle;
};

}
