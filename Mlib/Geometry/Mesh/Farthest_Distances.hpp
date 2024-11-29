#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <class TData, size_t tndim>
class PlaneNd;
class IIntersectableMesh;

struct VertexDistances {
    ScenePos min;
    ScenePos max;
};

VertexDistances get_farthest_distances(
    const IIntersectableMesh& mesh,
    const PlaneNd<ScenePos, 3>& plane);

}
