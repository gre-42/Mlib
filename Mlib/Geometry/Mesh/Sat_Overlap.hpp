#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <set>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TData, size_t... tshape>
class OrderableFixedArray;

// ScenePos get_overlap(
//     const CollisionTriangleSphere& t0,
//     const IIntersectableMesh& mesh1);

ScenePos sat_overlap_signed(
    const FixedArray<SceneDir, 3>& n,
    const std::set<OrderableFixedArray<CompressedScenePos, 3>>& vertices0,
    const std::set<OrderableFixedArray<CompressedScenePos, 3>>& vertices1);

void sat_overlap_unsigned(
    const FixedArray<SceneDir, 3>& l,
    const std::set<OrderableFixedArray<CompressedScenePos, 3>>& vertices0,
    const std::set<OrderableFixedArray<CompressedScenePos, 3>>& vertices1,
    ScenePos& overlap0,
    ScenePos& overlap1);

}
