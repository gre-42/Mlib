#pragma once
#include <cstddef>

namespace Mlib {

struct SceneGraphConfig {
    float max_distance_small = 1000.f;
    float max_distance_near_small = 400.f;
    float max_distance_near_large = 800.f;
    float max_distance_black = 100.f;
    size_t aggregate_update_interval = 100;
};

}
