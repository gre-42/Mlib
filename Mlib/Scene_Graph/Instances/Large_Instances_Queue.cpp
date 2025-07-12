#include "Large_Instances_Queue.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

LargeInstancesQueue::LargeInstancesQueue(ExternalRenderPassType render_pass)
    : render_pass_{ render_pass }
{
    if ((render_pass_ != ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC) &&
        (render_pass_ != ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC) &&
        (render_pass_ != ExternalRenderPassType::DIRTMAP) &&
        !any(render_pass_ & ExternalRenderPassType::STANDARD_MASK))
    {
        THROW_OR_ABORT("Unknown render pass");
    }
}

LargeInstancesQueue::~LargeInstancesQueue()
{}

void LargeInstancesQueue::insert(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& scvas,
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<float, ScenePos, 3>& m,
    const FixedArray<ScenePos, 3>& offset,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    InvisibilityHandling invisibility_handling)
{
    TransformationMatrix<float, float, 3> mo{m.R, (m.t - offset).casted<float>()};
    for (const auto& scva : scvas) {
        if (any(render_pass_ & ExternalRenderPassType::STANDARD_MASK)) {
            if (!VisibilityCheck{mvp}.is_visible(scva->name.full_name(), scva->material, scva->morphology, billboard_id, scene_graph_config, render_pass_)) {
                continue;
            }
        } else if (render_pass_ == ExternalRenderPassType::DIRTMAP) {
            continue;
        } else if (any(render_pass_ & ExternalRenderPassType::IS_GLOBAL_MASK)) {
            ExternalRenderPassType occluder_pass = (billboard_id != BILLBOARD_ID_NONE)
                ? scva->material.billboard_atlas_instance(billboard_id, scva->name.full_name()).occluder_pass
                : scva->material.occluder_pass;
            if (!any(occluder_pass & ExternalRenderPassType::IS_GLOBAL_MASK)) {
                if (invisibility_handling == InvisibilityHandling::SKIP) {
                    continue;
                } else {
                    THROW_OR_ABORT("Static instance has no occluder pass: \"" + scva->name.full_name() + '"');
                }
            }
            if ((occluder_pass & render_pass_) != render_pass_) {
                continue;
            }
        } else {
            THROW_OR_ABORT("Unsupported render pass: " + external_render_pass_type_to_string(render_pass_));
        }
        queue_.push_back(TransformedColoredVertexArray{
            .scva = scva,
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
