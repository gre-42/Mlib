#pragma once
#include <cstddef>

namespace Mlib {

template <class TData, size_t tndim>
class PlaneNd;
class IIntersectableMesh;

struct VertexDistances {
    double min;
    double max;
};

VertexDistances get_farthest_distances(
    const IIntersectableMesh& mesh,
    const PlaneNd<double, 3>& plane);

}
