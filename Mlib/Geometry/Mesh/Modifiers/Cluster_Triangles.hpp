#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Point_To_Grid_Center.hpp>
#include <Mlib/Iterator/Mapped_Iterator.hpp>
#include <functional>
#include <list>
#include <unordered_map>

namespace Mlib {

class GroupAndName;
template <class TPos>
struct PositionAndMeshes;

template <class TPos>
std::list<PositionAndMeshes<TPos>> cluster_triangles(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::function<FixedArray<TPos, 3>(const FixedArray<ColoredVertex<TPos>, 3>&)>& get_cluster_center,
    const GroupAndName& prefix);

template <class TPos, class TWidth>
inline std::function<FixedArray<TPos, 3>(const FixedArray<ColoredVertex<TPos>, 3>&)> cluster_center_by_grid(
    const FixedArray<TWidth, 3>& width)
{
    return [width](const FixedArray<ColoredVertex<TPos>, 3>& triangle){
        if (any(width == (TWidth)0.f)) {
            THROW_OR_ABORT("Cluster width is zero (1)");
        }
        auto m = [](const ColoredVertex<TPos>& e){ return &e.position; };
        auto s = BoundingSphere<TPos, 3>::from_iterator(
            MappedIterator{triangle.flat_begin(), m},
            MappedIterator{triangle.flat_end(), m});
        return point_to_grid_center(s.center, width);
    };
}

template <class TPos>
std::unordered_map<float, std::list<std::shared_ptr<ColoredVertexArray<TPos>>>>
    triangle_cluster_width_groups(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
{
    std::unordered_map<float, std::list<std::shared_ptr<ColoredVertexArray<TPos>>>> result;
    for (const auto& cva : cvas) {
        result[cva->morphology.triangle_cluster_width].push_back(cva);
    }
    return result;
}

}
