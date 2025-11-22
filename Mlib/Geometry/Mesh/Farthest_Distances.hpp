#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t tndim>
class PlaneNd;
class IIntersectableMesh;

struct VertexDistances {
    ScenePos min;
    ScenePos max;
};

VertexDistances get_farthest_distances(
    const IIntersectableMesh& mesh,
    const PlaneNd<SceneDir, CompressedScenePos, 3>& plane);

}
