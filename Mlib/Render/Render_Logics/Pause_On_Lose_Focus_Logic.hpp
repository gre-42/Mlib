#pragma once
#include <Mlib/Render/Render_Logic.hpp>

namespace Mlib {

class SetFps;
class Focuses;

class PauseOnLoseFocusLogic: public RenderLogic {
public:
    explicit PauseOnLoseFocusLogic(
        SetFps& set_fps,
        Focuses& focuses,
        Focus focus_mask);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
private:
    SetFps& set_fps_;
    Focuses& focuses_;
    Focus focus_mask_;
};

}
