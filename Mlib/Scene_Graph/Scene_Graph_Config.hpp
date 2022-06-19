#pragma once
#include <cstddef>

namespace Mlib {

struct SceneGraphConfig {
    float max_distance_black = 200.f;
    size_t small_aggregate_update_interval = 1 * 60;
    size_t large_aggregate_update_interval = 60 * 60;
};

}
