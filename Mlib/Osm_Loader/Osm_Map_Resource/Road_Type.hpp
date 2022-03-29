#pragma once
#include <compare>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace Mlib {

enum class RoadType {
    PATH,
    STREET,
    WALL
};

inline std::string road_type_to_string(RoadType st) {
    if (st == RoadType::PATH) {
        return "path";
    } else if (st == RoadType::STREET) {
        return "street";
    } else if (st == RoadType::WALL) {
        return "wall";
    } else {
        throw std::runtime_error("Unknown street type");
    }
}

inline std::string to_string(RoadType st) {
    return road_type_to_string(st);
}

struct RoadProperties {
    RoadType type;
    size_t nlanes;
    std::strong_ordering operator <=> (const RoadProperties&) const = default;
    inline explicit operator std::string () const {
        return road_type_to_string(type) + '_' + std::to_string(nlanes);
    }
};

}
