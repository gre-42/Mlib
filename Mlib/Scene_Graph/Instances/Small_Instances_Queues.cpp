#include "Small_Instances_Queues.hpp"
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Geometry/Mesh/Mesh_Meta.hpp>
#include <Mlib/Math/Transformation/Transformation_Variant.hpp>
#include <Mlib/Math/Transformation/Translation_Matrix.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Instances/Billboard_Container.hpp>
#include <Mlib/Scene_Graph/Instances/Deferred_Vertex_Arrays_And_Instances.hpp>
#include <Mlib/Scene_Graph/Instances/Extendable_Sortable_Vertex_Array_Instances.hpp>
#include <Mlib/Scene_Graph/Instances/Extendable_Vertex_Array_Instances.hpp>
#include <Mlib/Scene_Graph/Instances/Vertex_Data_And_Sorted_Instances.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Data.hpp>
#include <Mlib/Scene_Graph/Render/Sortable_Deferred_Gpu_Vertex_Data.hpp>
#include <Mlib/Testing/Assert.hpp>

using namespace Mlib;

SmallInstancesQueues::SmallInstancesQueues(
    ExternalRenderPassType main_render_pass,
    const std::set<ExternalRenderPassType>& black_render_passes)
    : main_render_pass_{main_render_pass}
{
    for (const auto& r : black_render_passes) {
        assert_true(!any(r & ExternalRenderPassType::STANDARD_MASK));
        black_queues_[r];
    }
}

SmallInstancesQueues::~SmallInstancesQueues() = default;

void SmallInstancesQueues::insert(
    const std::list<std::shared_ptr<IGpuVertexData>>& scvas,
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<SceneDir, ScenePos, 3>& m,
    const FixedArray<ScenePos, 3>& offset,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config)
{
    auto m_shifted = TranslationMatrix<ScenePos, 3>{-offset} * m;
    VisibilityCheck vc{ mvp };
    for (const auto& scva : scvas) {
        const auto& meta = scva->mesh_meta();
        if (vc.is_visible(meta.name.full_name_and_hash(), meta.material, meta.morphology, billboard_id, scene_graph_config, main_render_pass_))
        {
            auto m_shifted_i = instance_location_from_transformation(m_shifted, meta.material.transformation_mode);
            if (meta.material.blend_mode == BlendMode::INVISIBLE) {
                invisible_queue_[scva].insert(m_shifted_i, billboard_id);
            } else {
                auto key = SortableDeferredVertexData{
                    meta.material.blend_mode,
                    meta.material.continuous_blending_z_order,
                    meta.material.depth_func,
                    scva};
                standard_queue_[key].insert(m_shifted_i, (float)vc.sorting_key(meta.material), billboard_id);
            }
            for (auto& [rp, instances] : black_queues_) {
                assert_true(rp != main_render_pass_);
                if (vc.black_is_visible(
                    meta.name.full_name_and_hash(),
                    meta.material,
                    billboard_id,
                    scene_graph_config,
                    rp))
                {
                    instances[scva].insert(m_shifted_i, billboard_id);
                }
            }
        }
    }
}

std::map<ExternalRenderPassType, VertexDatasAndSortedInstances> SmallInstancesQueues::sorted_instances() const
{
    std::map<ExternalRenderPassType, VertexDatasAndSortedInstances> results;
    for (auto& [rp, lst] : black_queues_) {
        assert_true(rp != main_render_pass_);
        auto& dlst = results[rp];
        for (const auto& [a, instances] : lst) {
            auto& darray = dlst.emplace_back(a);
            darray.instances = instances.vectorized();
        }
    }
    {
        auto& dlst = results[main_render_pass_];
        for (const auto& [a, instances] : standard_queue_) {
            auto& darray = dlst.emplace_back(a.vertex_array);
            darray.instances = instances.sorted();
        }
    }
    for (auto& [rp, data] : results) {
        data.sort([](
            const VertexDataAndSortedInstances& a,
            const VertexDataAndSortedInstances& b)
            {
                return a.vertex_data->mesh_meta().material.rendering_sorting_key() < b.vertex_data->mesh_meta().material.rendering_sorting_key();
            });
    }
    return results;
}
