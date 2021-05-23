#include "Controls_Logic.hpp"
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

ControlsLogic::ControlsLogic(
    const std::string& gamepad_texture,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    UiFocus& ui_focus,
    size_t submenu_id)
: gamepad_texture_{gamepad_texture, ResourceUpdateCycle::ONCE, position, size, Focus::ALWAYS},
  ui_focus_{ui_focus},
  submenu_id_{submenu_id}
{}

void ControlsLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    if (ui_focus_.submenu_id == submenu_id_) {
        gamepad_texture_.render(
            width,
            height,
            render_config,
            scene_graph_config,
            render_results,
            frame_id);
    }
}

Focus ControlsLogic::focus_mask() const {
    return Focus::MENU;
}
