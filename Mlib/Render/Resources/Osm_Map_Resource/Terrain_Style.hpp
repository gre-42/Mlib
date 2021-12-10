#pragma once
#include <string>
#include <vector>

namespace Mlib {

struct ParsedResourceName;

struct TerrainStyle {
    std::vector<ParsedResourceName> near_resource_names;
    float min_near_distance_to_bdry = 0.f;
    float much_near_distance = INFINITY;
    bool is_small = true;
    inline bool is_visible() const {
        return !near_resource_names.empty() && (much_near_distance != INFINITY);
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(near_resource_names);
        archive(min_near_distance_to_bdry);
        archive(much_near_distance);
        archive(is_small);
    }
};

}
