#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

struct IntersectionScene;
template <class TDir, class TPos, size_t tndim>
class PlaneNd;
template <class TPosition>
struct CollisionRidgeSphere;
template <typename TData, size_t... tshape>
class FixedArray;

bool compute_edge_overlap(
    const IntersectionScene& c,
    const FixedArray<ScenePos, 3>& intersection_point,
    bool& sat_used,
    ScenePos& overlap,
    FixedArray<SceneDir, 3>& normal);

}
