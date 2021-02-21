#pragma once

namespace Mlib {

struct SceneGraphConfig {
    float max_distance_small = 1000;
    float min_distance_near = 1;
    float max_distance_near = 400;
    size_t aggregate_update_interval = 100;
};

}
