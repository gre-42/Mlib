#include "Cluster_Meshes.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Group_Meshes_By_Material.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Merge_Meshes.hpp>
#include <unordered_map>

using namespace Mlib;

template <class TPos>
std::list<MeshAndPosition<TPos>> Mlib::cluster_meshes(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::function<FixedArray<TPos, 3>(const ColoredVertexArray<TPos>&)>& get_cluster_center,
    const GroupAndName& name)
{
    std::unordered_map<OrderableFixedArray<TPos, 3>, std::list<std::shared_ptr<ColoredVertexArray<TPos>>>> clustered;
    for (const auto& cva : cvas) {
        auto center = get_cluster_center(*cva);
        clustered[OrderableFixedArray{center}].push_back(cva);
    }
    std::list<MeshAndPosition<TPos>> result;
    for (const auto& [center, cvas] : clustered) {
        for (const auto& [m, grouped_cvas] : group_meshes_by_material(cvas)) {
            result.emplace_back(merge_meshes(grouped_cvas, name, m.material, m.morphology), center);
        }
    }
    return result;
}

template std::list<MeshAndPosition<CompressedScenePos>> Mlib::cluster_meshes<CompressedScenePos>(
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    const std::function<FixedArray<CompressedScenePos, 3>(const ColoredVertexArray<CompressedScenePos>&)>& get_cluster_id,
    const GroupAndName& prefix);
