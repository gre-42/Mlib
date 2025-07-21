#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Point_To_Grid_Center.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <functional>
#include <list>
#include <memory>

namespace Mlib {

class GroupAndName;
template <class TPos>
struct PositionAndMeshes;

template <class TPos>
std::list<PositionAndMeshes<TPos>> cluster_meshes(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::function<FixedArray<TPos, 3>(const ColoredVertexArray<TPos>&)>& get_cluster_center,
    const GroupAndName& prefix);

template <class TPos, class TWidth>
inline std::function<FixedArray<TPos, 3>(const ColoredVertexArray<TPos>&)> cva_to_grid_center(
    const FixedArray<TWidth, 3>& width)
{
    return [width](const ColoredVertexArray<TPos>& cva){
        if (any(width == (TWidth)0.f)) {
            THROW_OR_ABORT("Cluster width is zero (0)");
        }
        return point_to_grid_center(cva.aabb().data().center(), width);
    };
}

template <class TPos>
std::unordered_map<float, std::list<std::shared_ptr<ColoredVertexArray<TPos>>>>
    object_cluster_width_groups(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
{
    std::unordered_map<float, std::list<std::shared_ptr<ColoredVertexArray<TPos>>>> result;
    for (const auto& cva : cvas) {
        result[cva->morphology.object_cluster_width].push_back(cva);
    }
    return result;
}

}
