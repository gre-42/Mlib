#include "Aggregate_Render_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IAggregate_Renderer.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IInstances_Renderer.hpp>
#include <mutex>

using namespace Mlib;

AggregateRenderLogic::AggregateRenderLogic(
    RenderingResources& rendering_resources,
    RenderLogic& child_logic)
    : child_logic_{ child_logic }
    , small_sorted_aggregate_renderer_{ std::make_shared<AggregateArrayRenderer>(rendering_resources) }
    , small_sorted_instances_renderers_{ std::make_shared<ArrayInstancesRenderers>(rendering_resources) }
    , large_aggregate_renderer_{ std::make_shared<AggregateArrayRenderer>(rendering_resources) }
    , large_instances_renderer_{ std::make_shared<ArrayInstancesRenderer>(rendering_resources) }
{}

AggregateRenderLogic::~AggregateRenderLogic() {
    on_destroy.clear();
}

void AggregateRenderLogic::init(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id)
{
    child_logic_.init(lx, ly, frame_id);
}

void AggregateRenderLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("AggregateRenderLogic::render");

    std::shared_lock lock{ mutex_ };

    bool create_render_guards = !any(frame_id.external_render_pass.pass & ExternalRenderPassType::IS_GLOBAL_MASK);
    std::optional<AggregateRendererGuard> arg;
    std::optional<InstancesRendererGuard> irg;
    if (create_render_guards) {
        arg.emplace(
            small_sorted_aggregate_renderer_,
            large_aggregate_renderer_);
        irg.emplace(
            small_sorted_instances_renderers_,
            large_instances_renderer_);
    }

    child_logic_.render(
        lx,
        ly,
        render_config,
        scene_graph_config,
        render_results,
        frame_id);
}

void AggregateRenderLogic::reset() {
    child_logic_.reset();
}

float AggregateRenderLogic::near_plane() const {
    return child_logic_.near_plane();
}

float AggregateRenderLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<ScenePos, 4, 4>& AggregateRenderLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, ScenePos, 3>& AggregateRenderLogic::iv() const {
    return child_logic_.iv();
}

DanglingPtr<const SceneNode> AggregateRenderLogic::camera_node() const {
    return child_logic_.camera_node();
}

bool AggregateRenderLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void AggregateRenderLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "AggregateRenderLogic\n";
    child_logic_.print(ostr, depth + 1);
}

void AggregateRenderLogic::invalidate_aggregate_renderers() {
    std::scoped_lock lock{ mutex_ };
    small_sorted_aggregate_renderer_->invalidate();
    small_sorted_instances_renderers_->invalidate();
    large_aggregate_renderer_->invalidate();
    large_instances_renderer_->invalidate();
}
