#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Point_To_Grid_Center.hpp>
#include <Mlib/Iterator/Mapped_Iterator.hpp>
#include <functional>
#include <list>

namespace Mlib {

template <class TPos>
struct ColoredVertex;
template <class TPos>
class ColoredVertexArray;
class GroupAndName;

template <class TPos>
std::list<std::shared_ptr<ColoredVertexArray<TPos>>> split_meshes(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::function<FixedArray<TPos, 3>(const FixedArray<ColoredVertex<TPos>, 3>&)>& get_cluster_center,
    const GroupAndName& prefix);

template <class TPos, class TWidth>
inline std::function<FixedArray<TPos, 3>(const FixedArray<ColoredVertex<TPos>, 3>&)> cluster_center_by_grid(
    const FixedArray<TWidth, 3>& width)
{
    return [width](const FixedArray<ColoredVertex<TPos>, 3>& triangle){
        auto m = [](const ColoredVertex<TPos>& e){ return &e.position; };
        auto s = BoundingSphere<TPos, 3>::from_iterator(
            MappedIterator{triangle.flat_begin(), m},
            MappedIterator{triangle.flat_end(), m});
        return point_to_grid_center(s.center, width);
    };
}

}
