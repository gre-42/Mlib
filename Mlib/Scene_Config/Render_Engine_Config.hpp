#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

struct RenderEngineConfig {
    // BVH; no grid, because the frustum is large.
    CompressedScenePos bvh_max_size = (CompressedScenePos)(2.f * meters);
    size_t bvh_levels = 17;
};

}
