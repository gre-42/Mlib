#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <optional>

namespace Mlib {

struct NormalAndOverlap {
    FixedArray<SceneDir, 3> normal = uninitialized;
    ScenePos overlap;
};

struct IntersectionInfo {
    std::optional<ScenePos> ray_t;
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    FixedArray<SceneDir, 3> normal0 = uninitialized;
    std::optional<NormalAndOverlap> no;
};

}
