#include "Cluster_Meshes.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Group_Meshes_By_Material.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Material_And_Morphology.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Merge_Meshes.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Position_And_Meshes.hpp>
#include <Mlib/Strings/Group_And_Name.hpp>
#include <unordered_map>

using namespace Mlib;

template <class TPos>
std::list<PositionAndMeshes<TPos>> Mlib::cluster_meshes(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::function<FixedArray<TPos, 3>(const ColoredVertexArray<TPos>&)>& get_cluster_center,
    const GroupAndName& prefix)
{
    std::unordered_map<OrderableFixedArray<TPos, 3>, std::list<std::shared_ptr<ColoredVertexArray<TPos>>>> clustered;
    for (const auto& cva : cvas) {
        auto center = get_cluster_center(*cva);
        clustered[make_orderable(center)].push_back(cva);
    }
    std::list<PositionAndMeshes<TPos>> result;
    size_t i = 0;
    for (const auto& [center, cvas] : clustered) {
        auto& c = result.emplace_back(center);
        for (const auto& [m, grouped_cvas] : group_meshes_by_material(cvas)) {
            auto& res = c.cvas[m.material.continuous_blending_z_order]
                .emplace_back(merge_meshes(grouped_cvas, prefix + std::to_string(i++), m.material, m.morphology));
            if (res->material.aggregate_mode != AggregateMode::NODE_OBJECT) {
                THROW_OR_ABORT("cluster_meshes: aggregate mode is not \"NODE_OBJECT\"");
            }
            res->material.aggregate_mode = AggregateMode::NONE;
        }
    }
    return result;
}

template std::list<PositionAndMeshes<float>> Mlib::cluster_meshes<float>(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas,
    const std::function<FixedArray<float, 3>(const ColoredVertexArray<float>&)>& get_cluster_id,
    const GroupAndName& prefix);

template std::list<PositionAndMeshes<CompressedScenePos>> Mlib::cluster_meshes<CompressedScenePos>(
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    const std::function<FixedArray<CompressedScenePos, 3>(const ColoredVertexArray<CompressedScenePos>&)>& get_cluster_id,
    const GroupAndName& prefix);
