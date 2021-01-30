#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <mutex>

namespace Mlib {

class LockingRenderLogic: public RenderLogic {
public:
    explicit LockingRenderLogic(
        RenderLogic& child_logic,
        std::recursive_mutex& mutex);
    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
private:
    RenderLogic& child_logic_;
    std::recursive_mutex& mutex_;
};

}
