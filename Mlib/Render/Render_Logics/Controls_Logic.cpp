#include "Controls_Logic.hpp"
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

ControlsLogic::ControlsLogic(
    const std::string& gamepad_texture,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    const FocusFilter& focus_filter)
: gamepad_texture_{ gamepad_texture, ResourceUpdateCycle::ONCE, position, size, {.focus_mask = Focus::ALWAYS} },
  focus_filter_{ focus_filter }
{}

void ControlsLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    gamepad_texture_.render(
        width,
        height,
        render_config,
        scene_graph_config,
        render_results,
        frame_id);
}

FocusFilter ControlsLogic::focus_filter() const {
    return focus_filter_;
}
