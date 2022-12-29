#pragma once
#include <compare>
#include <string>

namespace Mlib {

enum class RoadType {
    PATH,
    STREET,
    WALL
};

std::string road_type_to_string(RoadType st);

std::string to_string(RoadType st);

struct RoadProperties {
    RoadType type;
    size_t nlanes;
    std::strong_ordering operator <=> (const RoadProperties&) const = default;
    explicit operator std::string () const;
};

}
