#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

namespace Mlib {

class SetFps;
struct UiFocus;

class PauseOnLoseFocusLogic: public RenderLogic {
public:
    explicit PauseOnLoseFocusLogic(
        SetFps& set_fps,
        UiFocus& ui_focus,
        FocusFilter focus_filter);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
private:
    SetFps& set_fps_;
    UiFocus& ui_focus_;
    FocusFilter focus_filter_;
};

}
