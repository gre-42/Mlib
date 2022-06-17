#pragma once
#include <cstddef>

namespace Mlib {

struct SceneGraphConfig {
    float max_distance_small = 1000.f;
    float max_distance_near_small = 400.f;
    float max_distance_near_large = 800.f;
    float max_distance_black = 200.f;
    size_t small_aggregate_update_interval = 1 * 60;
    size_t large_aggregate_update_interval = 60 * 60;
};

}
