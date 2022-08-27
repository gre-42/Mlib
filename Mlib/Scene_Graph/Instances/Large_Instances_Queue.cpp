#include "Large_Instances_Queue.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Visibility_Check.hpp>

using namespace Mlib;

LargeInstancesQueue::LargeInstancesQueue(ExternalRenderPassType render_pass)
: render_pass_{render_pass}
{
    assert_true((render_pass_ == ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC) ||
                (render_pass_ == ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC) ||
                (render_pass_ == ExternalRenderPassType::DIRTMAP) ||
                (render_pass_ == ExternalRenderPassType::STANDARD));
}

LargeInstancesQueue::~LargeInstancesQueue()
{}

void LargeInstancesQueue::insert(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& scvas,
    const FixedArray<double, 4, 4>& mvp,
    const TransformationMatrix<float, double, 3>& m,
    const FixedArray<double, 3>& offset,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config,
    InvisibilityHandling invisibility_handling)
{
    TransformationMatrix<float, float, 3> mo{m.R(), (m.t() - offset).casted<float>()};
    for (const auto& cva : scvas) {
        if (render_pass_ == ExternalRenderPassType::STANDARD) {
            if (!VisibilityCheck{mvp}.is_visible(cva->material, billboard_id, scene_graph_config, render_pass_)) {
                continue;
            }
        } else if (render_pass_ == ExternalRenderPassType::DIRTMAP) {
            continue;
        } else if (any(render_pass_ & ExternalRenderPassType::IS_STATIC_MASK)) {
            ExternalRenderPassType occluder_pass = (billboard_id != UINT32_MAX)
                ? cva->material.billboard_atlas_instance(billboard_id).occluder_pass
                : cva->material.occluder_pass;
            if (!any(occluder_pass & ExternalRenderPassType::IS_STATIC_MASK)) {
                if (invisibility_handling == InvisibilityHandling::SKIP) {
                    continue;
                } else {
                    throw std::runtime_error("Static instance has no occluder pass: \"" + cva->name + '"');
                }
            }
            if ((occluder_pass & render_pass_) != render_pass_) {
                continue;
            }
        } else {
            throw std::runtime_error("Unsupported render pass: " + external_render_pass_type_to_string(render_pass_));
        }
        queue_.push_back(TransformedColoredVertexArray{
            .cva = cva,
            .trafo = TransformationAndBillboardId{
                .transformation_matrix = mo,
                .billboard_id = billboard_id}});
    }
}

const std::list<TransformedColoredVertexArray>& LargeInstancesQueue::queue() const {
    return queue_;
}

ExternalRenderPassType LargeInstancesQueue::render_pass() const {
    return render_pass_;
}
