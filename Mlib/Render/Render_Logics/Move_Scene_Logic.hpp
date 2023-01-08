#pragma once
#include <Mlib/Render/Render_Logic.hpp>

namespace Mlib {

class Scene;
class DeleteNodeMutex;

class MoveSceneLogic: public RenderLogic {
public:
    explicit MoveSceneLogic(
        Scene& scene,
        DeleteNodeMutex& delete_node_mutex,
        float speed = 1);

    virtual void render(
        int width,
        int height,
        float xdpi,
        float ydpi,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    Scene& scene_;
    bool deleter_thread_set_;
    DeleteNodeMutex& delete_node_mutex_;
    float speed_;
};

}
