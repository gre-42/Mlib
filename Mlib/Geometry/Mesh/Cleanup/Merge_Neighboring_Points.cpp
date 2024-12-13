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
    PointWithoutPayloadVectorBvh<TPos, 3>& bvh,
    const TPos& max_distance)
{
    // linfo() << "Merging: " << cva.name;
    bool warning_printed = false;
    for (auto& tri : cva.triangles) {
        for (auto& v : tri.flat_iterable()) {
            const FixedArray<TPos, 3>* neighbor;
            auto min_dist2 = bvh.min_distance(
                v.position,
                max_distance,
                [&v](const auto& a){return sum(squared(a - v.position));},
                &neighbor);
            if (min_dist2.has_value() && (min_dist2 <= squared(max_distance)))
            {
                if (any(v.position != *neighbor)) {
                    if (!warning_printed) {
                        linfo() << "Merging points at " << v.position << ". Further warnings suppressed.";
                        warning_printed = true;
                    }
                    v.position = *neighbor;
                }
            } else {
                bvh.insert(PointWithoutPayload{ v.position });
            }
        }
    }
    // linfo() << "Merging done: " << cva.name;
}

template void Mlib::merge_neighboring_points<float>(ColoredVertexArray<float>& cva, PointWithoutPayloadVectorBvh<float, 3>& bvh, const float& min_distance);
template void Mlib::merge_neighboring_points<CompressedScenePos>(ColoredVertexArray<CompressedScenePos>& cva, PointWithoutPayloadVectorBvh<CompressedScenePos, 3>& bvh, const CompressedScenePos& min_distance);
