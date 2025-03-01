#include "Controls_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>

using namespace Mlib;

ControlsLogic::ControlsLogic(
    ColormapWithModifiers image_resource_name,
    std::unique_ptr<IWidget>&& widget,
    DelayLoadPolicy delay_load_policy,
    FocusFilter focus_filter)
: gamepad_texture_{
    std::make_shared<FillWithTextureLogic>(
        RenderingContextStack::primary_rendering_resources().get_texture_lazy(image_resource_name)),
    std::move(widget),
    delay_load_policy,
    {.focus_mask = Focus::ALWAYS} },
  focus_filter_{ std::move(focus_filter) }
{}

ControlsLogic::~ControlsLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> ControlsLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return gamepad_texture_.try_render_setup(lx, ly, frame_id);
}

bool ControlsLogic::render_optional_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup* setup)
{
    LOG_FUNCTION("ControlsLogic::render");
    gamepad_texture_.render_auto_setup(
        lx,
        ly,
        render_config,
        scene_graph_config,
        render_results,
        frame_id,
        setup);
    return true;
}

FocusFilter ControlsLogic::focus_filter() const {
    return focus_filter_;
}

void ControlsLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ControlsLogic\n";
}
