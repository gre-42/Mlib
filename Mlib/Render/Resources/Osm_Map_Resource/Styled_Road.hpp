#pragma once
#include <Mlib/Render/Resources/Osm_Map_Resource/Road_Type.hpp>
#include <memory>

namespace Mlib {

struct RoadProperties;
class TriangleList;

struct StyledRoad {
    std::shared_ptr<TriangleList> triangle_list;
    float uvx;
    std::partial_ordering operator <=> (const StyledRoad&) const = default;
};

struct StyledRoadEntry {
    RoadProperties road_properties;
    StyledRoad styled_road;
    std::partial_ordering operator <=> (const StyledRoadEntry&) const = default;
};

}
