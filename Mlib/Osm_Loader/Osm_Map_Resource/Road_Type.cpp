#include "Road_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

std::string Mlib::road_type_to_string(RoadType st) {
    switch (st) {
    case RoadType::NONE:
        return "none";
    case RoadType::PATH:
        return "path";
    case RoadType::STREET:
        return "street";
    case RoadType::TAXIWAY:
        return "taxiway";
    case RoadType::RUNWAY:
        return "runway";
    case RoadType::RUNWAY_DISPLACEMENT_THRESHOLD:
        return "runway_displacement_threshold";
    case RoadType::WALL:
        return "wall";
    case RoadType::ANY_PLANE_ROAD:
        return "any_plane_road";
    }
    THROW_OR_ABORT("Unknown street type");
}

std::string Mlib::to_string(RoadType st) {
    return road_type_to_string(st);
}

RoadProperties::operator std::string () const {
    return road_type_to_string(type) + '_' + std::to_string(nlanes);
}
