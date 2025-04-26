#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Point_To_Grid_Center.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <functional>
#include <list>
#include <memory>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
class GroupAndName;
template <class TPos>
struct MeshAndPosition;

template <class TPos>
std::list<MeshAndPosition<TPos>> cluster_meshes(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::function<FixedArray<TPos, 3>(const ColoredVertexArray<TPos>&)>& get_cluster_center,
    const GroupAndName& prefix);

template <class TPos, class TWidth>
inline std::function<FixedArray<TPos, 3>(const ColoredVertexArray<TPos>&)> cva_to_grid_center(
    const FixedArray<TWidth, 3>& width)
{
    return [width](const ColoredVertexArray<TPos>& cva){
        return point_to_grid_center(cva.aabb().data().center(), width);
    };
}

}
