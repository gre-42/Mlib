#include "Controls_Logic.hpp"
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

ControlsLogic::ControlsLogic(
    const std::string& gamepad_texture,
    std::unique_ptr<IWidget>&& widget,
    DelayLoadPolicy delay_load_policy,
    FocusFilter focus_filter)
: gamepad_texture_{
    std::make_shared<FillWithTextureLogic>(
        RenderingContextStack::primary_rendering_resources(),
        gamepad_texture,
        ResourceUpdateCycle::ONCE,
        ColorMode::RGBA),
    std::move(widget),
    delay_load_policy,
    {.focus_mask = Focus::ALWAYS} },
  focus_filter_{ std::move(focus_filter) }
{}

ControlsLogic::~ControlsLogic() = default;

void ControlsLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("ControlsLogic::render");
    gamepad_texture_.render(
        lx,
        ly,
        render_config,
        scene_graph_config,
        render_results,
        frame_id);
}

FocusFilter ControlsLogic::focus_filter() const {
    return focus_filter_;
}

void ControlsLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ControlsLogic\n";
}
