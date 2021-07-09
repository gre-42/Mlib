#pragma once
#include <string>
#include <vector>

namespace Mlib {

struct TerrainStyle {
    std::vector<std::string> near_resource_names;
    float much_near_distance = INFINITY;
    bool is_small = true;
    inline bool is_visible() const {
        return !near_resource_names.empty() && (much_near_distance != INFINITY);
    }
};

}
