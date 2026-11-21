#include "Road_Type.hpp"
#include <stdexcept>

using namespace std::string_view_literals;
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
    throw std::runtime_error("Unknown street type");
}

RoadType Mlib::road_type_from_string(std::string_view s) {
    static const std::unordered_map<std::string_view, RoadType> m{
        {"path"sv, RoadType::PATH},
        {"street"sv, RoadType::STREET},
        {"taxiway"sv, RoadType::TAXIWAY},
        {"runway"sv, RoadType::RUNWAY},
        {"runway_displacement_threshold"sv, RoadType::RUNWAY_DISPLACEMENT_THRESHOLD},
        {"wall"sv, RoadType::WALL},
        {"any_plane_road"sv, RoadType::ANY_PLANE_ROAD}};
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown road type: \"" + std::string{ s } + '"');
    }
    return it->second;
}

std::string Mlib::to_string(RoadType st) {
    return road_type_to_string(st);
}

RoadProperties::operator std::string () const {
    return road_type_to_string(type) + '_' + std::to_string(nlanes);
}
