
#include "Large_Instances_Queue.hpp"
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Math/Transformation/Transformation_Variant.hpp>
#include <Mlib/Math/Transformation/Translation_Matrix.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Instances/Billboard_Container.hpp>
#include <Mlib/Scene_Graph/Instances/Extendable_Vertex_Array_Instances.hpp>
#include <Mlib/Scene_Graph/Instances/Sorted_Vertex_Array_Instances.hpp>
#include <Mlib/Scene_Graph/Instances/Vertex_Data_And_Sorted_Instances.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Data.hpp>
#include <Mlib/Testing/Assert.hpp>
#include <stdexcept>

using namespace Mlib;

LargeInstancesQueue::LargeInstancesQueue(ExternalRenderPassType render_pass)
    : render_pass_{ render_pass }
{
    if ((render_pass_ != ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC) &&
        (render_pass_ != ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC) &&
        (render_pass_ != ExternalRenderPassType::DIRTMAP) &&
        (render_pass_ != ExternalRenderPassType::BILLBOARD_SCENE) &&
        !any(render_pass_ & ExternalRenderPassType::STANDARD_MASK))
    {
        throw std::runtime_error("Unknown render pass");
    }
}

LargeInstancesQueue::~LargeInstancesQueue() = default;

void LargeInstancesQueue::insert(
    const std::list<std::shared_ptr<IGpuVertexData>>& scvas,
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<SceneDir, ScenePos, 3>& m,
    const FixedArray<ScenePos, 3>& offset,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    InvisibilityHandling invisibility_handling)
{
    auto m_shifted = TranslationMatrix<ScenePos, 3>{-offset} * m;
    for (const auto& scva : scvas) {
        const auto& meta = scva->mesh_meta();
        if (any(render_pass_ & (ExternalRenderPassType::STANDARD_MASK | ExternalRenderPassType::BILLBOARD_SCENE))) {
            if (!VisibilityCheck{mvp}.is_visible(meta.name.full_name_and_hash(), meta.material, meta.morphology, billboard_id, scene_graph_config, render_pass_)) {
                continue;
            }
        } else if (render_pass_ == ExternalRenderPassType::DIRTMAP) {
            continue;
        } else if (any(render_pass_ & ExternalRenderPassType::IS_GLOBAL_MASK)) {
            ExternalRenderPassType occluder_pass = (billboard_id != BILLBOARD_ID_NONE)
                ? meta.material.billboard_atlas_instance(billboard_id, meta.name.full_name()).occluder_pass
                : meta.material.occluder_pass;
            if (!any(occluder_pass & ExternalRenderPassType::IS_GLOBAL_MASK)) {
                if (invisibility_handling == InvisibilityHandling::SKIP) {
                    continue;
                } else {
                    throw std::runtime_error("Static instance has no occluder pass: \"" + meta.name.full_name() + '"');
                }
            }
            if ((occluder_pass & render_pass_) != render_pass_) {
                continue;
            }
        } else {
            throw std::runtime_error("Unsupported render pass: " + external_render_pass_type_to_string(render_pass_));
        }
        auto m_shifted_i = instance_location_from_transformation(m_shifted, meta.material.transformation_mode);
        queue_[scva].insert(m_shifted_i, billboard_id);
    }
}

VertexDatasAndSortedInstances LargeInstancesQueue::queue() const {
    VertexDatasAndSortedInstances result;
    for (const auto& [a, instances] : queue_) {
        result.emplace_back(a, instances.vectorized());
    }
    result.sort([](
        const VertexDataAndSortedInstances& a,
        const VertexDataAndSortedInstances& b)
        {
            return a.vertex_data->mesh_meta().material.rendering_sorting_key() < b.vertex_data->mesh_meta().material.rendering_sorting_key();
        });
    return result;
}

ExternalRenderPassType LargeInstancesQueue::render_pass() const {
    return render_pass_;
}
