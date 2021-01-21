#include "Pause_On_Lose_Focus_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Set_Fps.hpp>

using namespace Mlib;

PauseOnLoseFocusLogic::PauseOnLoseFocusLogic(
    SetFps& set_fps,
    Focuses& focuses,
    Focus focus_mask)
: set_fps_{set_fps},
  focuses_{focuses},
  focus_mask_{focus_mask}
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

    if ((focuses_.focus() & focus_mask_) != Focus::NONE) {
        set_fps_.resume();
    } else {
        set_fps_.pause();
    }
}
