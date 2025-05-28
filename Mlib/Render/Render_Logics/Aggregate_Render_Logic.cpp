#include "Aggregate_Render_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IAggregate_Renderer.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IInstances_Renderer.hpp>
#include <mutex>

using namespace Mlib;

AggregateRenderLogic::AggregateRenderLogic(
    RenderingResources& rendering_resources,
    RenderLogic& child_logic)
    : child_logic_{ child_logic }
    , small_aggregate_bg_worker_{ "Small_agg_BG" }
    , large_aggregate_bg_worker_{ "Large_agg_BG" }
    , small_instances_bg_worker_{ "Small_inst_BG" }
    , large_instances_bg_worker_{ "Large_inst_BG" }
    , small_sorted_aggregate_renderer_{ std::make_shared<AggregateArrayRenderer>(rendering_resources) }
    , small_sorted_instances_renderers_{ std::make_shared<ArrayInstancesRenderers>(rendering_resources) }
    , large_aggregate_renderer_{ std::make_shared<AggregateArrayRenderer>(rendering_resources) }
    , large_instances_renderer_{ std::make_shared<ArrayInstancesRenderer>(rendering_resources) }
{}

AggregateRenderLogic::~AggregateRenderLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> AggregateRenderLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return child_logic_.render_setup(lx, ly, frame_id);
}

bool AggregateRenderLogic::render_optional_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup* setup)
{
    LOG_FUNCTION("AggregateRenderLogic::render");

    std::shared_lock lock{ mutex_ };

    bool create_render_guards = !any(frame_id.external_render_pass.pass & ExternalRenderPassType::IS_GLOBAL_MASK);
    std::optional<AggregateRendererGuard> arg;
    std::optional<InstancesRendererGuard> irg;
    if (create_render_guards) {
        arg.emplace(
            &small_aggregate_bg_worker_,
            &large_aggregate_bg_worker_,
            small_sorted_aggregate_renderer_,
            large_aggregate_renderer_);
        irg.emplace(
            &small_instances_bg_worker_,
            &large_instances_bg_worker_,
            small_sorted_instances_renderers_,
            large_instances_renderer_);
    }

    child_logic_.render_auto_setup(
        lx,
        ly,
        render_config,
        scene_graph_config,
        render_results,
        frame_id,
        setup);
    return true;
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

void AggregateRenderLogic::wait_until_done() {
    large_aggregate_bg_worker_.wait_until_done();
    large_instances_bg_worker_.wait_until_done();
    small_aggregate_bg_worker_.wait_until_done();
    small_instances_bg_worker_.wait_until_done();
}

void AggregateRenderLogic::stop_and_join() {
    large_aggregate_bg_worker_.shutdown();
    large_instances_bg_worker_.shutdown();
    small_aggregate_bg_worker_.shutdown();
    small_instances_bg_worker_.shutdown();
}
