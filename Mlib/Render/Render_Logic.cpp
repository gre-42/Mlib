#include "Render_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Render/Render_Setup.hpp>

using namespace Mlib;

RenderLogic::RenderLogic() = default;

RenderLogic::~RenderLogic() = default;

RenderSetup RenderLogic::render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    auto result = try_render_setup(lx, ly, frame_id);
    if (!result.has_value()) {
        THROW_OR_ABORT("RenderLogic::render_setup: call to try_render_setup returned nullopt");
    }
    return std::move(*result);
}

void RenderLogic::render_auto_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup* setup)
{
    if (render_optional_setup(lx, ly, render_config, scene_graph_config, render_results, frame_id, setup)) {
        return;
    }
    if (setup != nullptr) {
        render_with_setup(lx, ly, render_config, scene_graph_config, render_results, frame_id, *setup);
    } else {
        render_without_setup(lx, ly, render_config, scene_graph_config, render_results, frame_id);
    }
}

void RenderLogic::render_toplevel(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    auto setup = try_render_setup(lx, ly, frame_id);
    if (setup.has_value()) {
        render_with_setup(lx, ly, render_config, scene_graph_config, render_results, frame_id, *setup);
    } else {
        render_without_setup(lx, ly, render_config, scene_graph_config, render_results, frame_id);
    }
}

void RenderLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    if (!render_optional_setup(lx, ly, render_config, scene_graph_config, render_results, frame_id, nullptr)) {
        THROW_OR_ABORT("RenderLogic::render_without_setup not implemented");
    }
}

void RenderLogic::render_with_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup& setup)
{
    if (!render_optional_setup(lx, ly, render_config, scene_graph_config, render_results, frame_id, &setup)) {
        THROW_OR_ABORT("RenderLogic::render_with_setup not implemented");
    }
}

bool RenderLogic::render_optional_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup* setup)
{
    return false;
}

FocusFilter RenderLogic::focus_filter() const {
    return { .focus_mask = Focus::ALWAYS };
}
