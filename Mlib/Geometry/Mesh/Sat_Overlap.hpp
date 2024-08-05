#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>
#include <set>

namespace Mlib {

template <class TData, size_t... tshape>
class FixedArray;
template <class TData, size_t tshape0, size_t... tshape>
class OrderableFixedArray;

// ScenePos get_overlap(
//     const CollisionTriangleSphere& t0,
//     const IIntersectableMesh& mesh1);

ScenePos sat_overlap_signed(
    const FixedArray<ScenePos, 3>& n,
    const std::set<OrderableFixedArray<ScenePos, 3>>& vertices0,
    const std::set<OrderableFixedArray<ScenePos, 3>>& vertices1);

void sat_overlap_unsigned(
    const FixedArray<ScenePos, 3>& l,
    const std::set<OrderableFixedArray<ScenePos, 3>>& vertices0,
    const std::set<OrderableFixedArray<ScenePos, 3>>& vertices1,
    ScenePos& overlap0,
    ScenePos& overlap1);

}
