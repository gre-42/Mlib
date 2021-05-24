#include "Pause_On_Lose_Focus_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Set_Fps.hpp>

using namespace Mlib;

PauseOnLoseFocusLogic::PauseOnLoseFocusLogic(
    SetFps& set_fps,
    UiFocus& ui_focus,
    FocusFilter focus_filter)
: set_fps_{ set_fps },
  ui_focus_{ ui_focus },
  focus_filter_{ focus_filter }
{}

void PauseOnLoseFocusLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("PauseOnLoseFocusLogic::render");

    if (ui_focus_.has_focus(focus_filter_)) {
        set_fps_.resume();
    } else {
        set_fps_.pause();
    }
}
