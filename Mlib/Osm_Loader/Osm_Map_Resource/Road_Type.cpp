#include "Road_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

std::string Mlib::road_type_to_string(RoadType st) {
    if (st == RoadType::PATH) {
        return "path";
    } else if (st == RoadType::STREET) {
        return "street";
    } else if (st == RoadType::WALL) {
        return "wall";
    } else {
        THROW_OR_ABORT("Unknown street type");
    }
}

std::string Mlib::to_string(RoadType st) {
    return road_type_to_string(st);
}

RoadProperties::operator std::string () const {
    return road_type_to_string(type) + '_' + std::to_string(nlanes);
}
