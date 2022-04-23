#include "Pause_On_Lose_Focus_Logic.hpp"
#include <Mlib/Fps/Set_Fps.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

PauseOnLoseFocusLogic::PauseOnLoseFocusLogic(
    std::atomic_bool& audio_paused,
    SetFps& set_fps,
    UiFocus& ui_focus,
    FocusFilter focus_filter)
: audio_paused_{ audio_paused },
  set_fps_{ set_fps },
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
        audio_paused_ = false;
    } else {
        set_fps_.pause();
        audio_paused_ = true;
    }
}

void PauseOnLoseFocusLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "PauseOnLoseFocusLogic\n";
}
