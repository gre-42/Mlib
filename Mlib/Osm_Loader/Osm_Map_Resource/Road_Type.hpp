#pragma once
#include <compare>
#include <string>

namespace Mlib {

enum class RoadType {
    NONE = 0,
    PATH = 1 << 0,
    STREET = 1 << 1,
    TAXIWAY = 1 << 2,
    RUNWAY = 1 << 3,
    RUNWAY_DISPLACEMENT_THRESHOLD = 1 << 4,
    WALL = 1 << 5,
    ANY_PLANE_ROAD = TAXIWAY | RUNWAY
};

static inline bool any(RoadType a) {
    return a != RoadType::NONE;
}

static inline RoadType operator & (RoadType a, RoadType b) {
    return (RoadType)((int)a & (int)b);
}

std::string road_type_to_string(RoadType st);

std::string to_string(RoadType st);

struct RoadProperties {
    RoadType type;
    size_t nlanes;
    std::strong_ordering operator <=> (const RoadProperties&) const = default;
    explicit operator std::string () const;
};

}
