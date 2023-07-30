#include "Merge_Neighboring_Points.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Largest_Cosine.hpp>
#include <list>

using namespace Mlib;

template <class TPos>
void Mlib::merge_neighboring_points(
    ColoredVertexArray<TPos>& cva,
    Bvh<TPos, FixedArray<TPos, 3>, 3>& bvh,
    const TPos& max_distance)
{
    // linfo() << "Merging: " << cva.name;
    for (auto& tri : cva.triangles) {
        for (auto& v : tri.flat_iterable()) {
            const FixedArray<TPos, 3>* neighbor;
            auto min_dist2 = bvh.min_distance(
                v.position,
                max_distance,
                [&v](const auto& a){return sum(squared(a - v.position));},
                &neighbor);
            if (min_dist2 <= squared(max_distance))
            {
                if (any(v.position != *neighbor)) {
                    linfo() << "Merging points at " << v.position;
                    v.position = *neighbor;
                }
            } else {
                bvh.insert(AxisAlignedBoundingBox{v.position}, v.position);
            }
        }
    }
    // linfo() << "Merging done: " << cva.name;
}

namespace Mlib {
    template void merge_neighboring_points<float>(ColoredVertexArray<float>& cva, Bvh<float, FixedArray<float, 3>, 3>& bvh, const float& min_distance);
    template void merge_neighboring_points<double>(ColoredVertexArray<double>& cva, Bvh<double, FixedArray<double, 3>, 3>& bvh, const double& min_distance);
}
