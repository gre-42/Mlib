#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <atomic>

namespace Mlib {

class PhysicsEngine;
class SetFps;
struct UiFocus;

class PauseOnLoseFocusLogic: public RenderLogic {
public:
    explicit PauseOnLoseFocusLogic(
        std::atomic_bool& audio_paused,
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
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    std::atomic_bool& audio_paused_;
    SetFps& set_fps_;
    UiFocus& ui_focus_;
    FocusFilter focus_filter_;
};

}
