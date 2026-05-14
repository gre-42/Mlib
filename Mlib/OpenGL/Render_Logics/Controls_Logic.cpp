#include "Controls_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Misc/Log.hpp>
#include <Mlib/OpenGL/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/OpenGL/Render_Setup.hpp>
#include <Mlib/Resource_Context/Rendering_Context.hpp>

using namespace Mlib;

ControlsLogic::ControlsLogic(
    std::shared_ptr<ITextureHandle> texture,
    std::unique_ptr<IWidget>&& widget,
    DelayLoadPolicy delay_load_policy,
    FocusFilter focus_filter)
: gamepad_texture_{
    nullptr, // object pool
    nullptr, // player
    std::make_shared<FillWithTextureLogic>(texture),
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

bool ControlsLogic::is_visible(const UiFocus& ui_focus) const {
    std::shared_lock lock{ ui_focus.focuses.mutex };
    return ui_focus.has_focus(focus_filter_);
}

void ControlsLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ControlsLogic";
}
