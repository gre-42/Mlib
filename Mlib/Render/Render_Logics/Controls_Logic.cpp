#include "Controls_Logic.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

ControlsLogic::ControlsLogic(
    const std::string& gamepad_texture,
    std::unique_ptr<IWidget>&& widget,
    FocusFilter focus_filter)
: gamepad_texture_{ gamepad_texture, std::move(widget), ResourceUpdateCycle::ONCE, {.focus_mask = Focus::ALWAYS} },
  focus_filter_{ std::move(focus_filter) }
{}

ControlsLogic::~ControlsLogic() = default;

void ControlsLogic::render(
    int width,
    int height,
    float xdpi,
    float ydpi,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("ControlsLogic::render");
    gamepad_texture_.render(
        width,
        height,
        xdpi,
        ydpi,
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
