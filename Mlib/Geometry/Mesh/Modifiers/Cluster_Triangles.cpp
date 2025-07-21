#include "Cluster_Triangles.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Material_And_Morphology.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Merge_Meshes.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Position_And_Meshes.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Strings/Group_And_Name.hpp>
#include <unordered_map>

using namespace Mlib;

template <class TPos>
struct Clustered {
    std::list<FixedArray<ColoredVertex<TPos>, 3>> triangles;
    std::list<FixedArray<uint8_t, 3>> discrete_triangle_texture_layers;
    std::list<FixedArray<float, 3>> alpha;
    std::list<FixedArray<float, 4>> interiormap_uvmaps;
};

template <class TPos>
std::list<PositionAndMeshes<TPos>> Mlib::cluster_triangles(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::function<FixedArray<TPos, 3>(const FixedArray<ColoredVertex<TPos>, 3>&)>& get_cluster_center,
    const GroupAndName& prefix)
{
    std::unordered_map<OrderableFixedArray<TPos, 3>, std::map<MaterialAndMorphology, Clustered<TPos>>> clustered;
    for (const auto& cva : cvas) {
        if (!cva->discrete_triangle_texture_layers.empty() &&
            (cva->discrete_triangle_texture_layers.size() != cva->triangles.size()))
        {
            THROW_OR_ABORT("cluster_triangles: discrete_triangle_texture_layers size mismatch");
        }
        if (!cva->alpha.empty() &&
            (cva->alpha.size() != cva->triangles.size()))
        {
            THROW_OR_ABORT("cluster_triangles: alpha size mismatch");
        }
        if (!cva->interiormap_uvmaps.empty() &&
            (cva->interiormap_uvmaps.size() != cva->triangles.size()))
        {
            THROW_OR_ABORT("cluster_triangles: interiormap_uvmaps size mismatch");
        }
        if (cva->modifier_backlog != ModifierBacklog{}) {
            THROW_OR_ABORT("cluster_triangles: modifier_backlog is not empty: " + modifier_backlog_to_string(cva->modifier_backlog));
        }
        auto mm = MaterialAndMorphology{cva->material, cva->morphology};
        for (const auto& [i, tri] : enumerate(cva->triangles)) {
            auto center = get_cluster_center(*tri);
            auto& cluster = clustered[make_orderable(center)][mm];
            cluster.triangles.push_back(tri);
            if (!cva->discrete_triangle_texture_layers.empty()) {
                cluster.discrete_triangle_texture_layers.push_back(cva->discrete_triangle_texture_layers[i]);
            }
            if (!cva->alpha.empty()) {
                cluster.alpha.push_back(cva->alpha[i]);
            }
            if (!cva->interiormap_uvmaps.empty()) {
                cluster.interiormap_uvmaps.push_back(cva->interiormap_uvmaps[i]);
            }
        }
    }
    size_t i = 0;
    std::list<PositionAndMeshes<TPos>> result;
    for (const auto& [center, lists] : clustered) {
        auto& pm = result.emplace_back(center);
        for (const auto& [mm, c] : lists) {
            auto& ccva = pm.cvas.emplace_back(
                std::make_shared<ColoredVertexArray<TPos>>(
                    prefix + std::to_string(i++),
                    mm.material,
                    mm.morphology,
                    ModifierBacklog{},
                    UUVector<FixedArray<ColoredVertex<TPos>, 4>>{},
                    UUVector<FixedArray<ColoredVertex<TPos>, 3>>(c.triangles.begin(), c.triangles.end()),
                    UUVector<FixedArray<ColoredVertex<TPos>, 2>>{},
                    UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},
                    UUVector<FixedArray<float, 3>>{},
                    UUVector<FixedArray<uint8_t, 3>>(c.discrete_triangle_texture_layers.begin(), c.discrete_triangle_texture_layers.end()),
                    std::vector<UUVector<FixedArray<float, 3, 2>>>{},
                    std::vector<UUVector<FixedArray<float, 3>>>{},
                    UUVector<FixedArray<float, 3>>(c.alpha.begin(), c.alpha.end()),
                    UUVector<FixedArray<float, 4>>(c.interiormap_uvmaps.begin(), c.interiormap_uvmaps.end())));
            if (ccva->material.aggregate_mode != AggregateMode::NODE_TRIANGLES) {
                THROW_OR_ABORT("cluster_triangles: aggregate mode is not \"NODE_TRIANGLES\"");
            }
            ccva->material.aggregate_mode = AggregateMode::NONE;
        }
    }
    return result;
}

template std::list<PositionAndMeshes<float>> Mlib::cluster_triangles<float>(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas,
    const std::function<FixedArray<float, 3>(const FixedArray<ColoredVertex<float>, 3>&)>& get_cluster_id,
    const GroupAndName& prefix);

template std::list<PositionAndMeshes<CompressedScenePos>> Mlib::cluster_triangles<CompressedScenePos>(
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    const std::function<FixedArray<CompressedScenePos, 3>(const FixedArray<ColoredVertex<CompressedScenePos>, 3>&)>& get_cluster_id,
    const GroupAndName& prefix);
