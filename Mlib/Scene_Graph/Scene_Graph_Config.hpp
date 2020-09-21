#pragma once

namespace Mlib {

struct SceneGraphConfig {
    float min_distance_small = 1;
    float max_distance_small = 1000;
    size_t aggregate_update_interval = 100;
};

}
