#pragma once
#include <Mlib/Physics/Units.hpp>
#include <cstddef>

namespace Mlib {

class IRenderableHider;

struct SceneGraphConfig {
    float max_distance_black = 200.f * meters;
    size_t small_aggregate_update_interval = 1 * 60;
    float large_max_offset_deviation = 200.f * meters;
    IRenderableHider* renderable_hider = nullptr;
};

}
