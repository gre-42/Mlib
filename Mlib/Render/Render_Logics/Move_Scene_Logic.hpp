#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <chrono>

namespace Mlib {

class Scene;
class DeleteNodeMutex;

class MoveSceneLogic final: public RenderLogic {
public:
    explicit MoveSceneLogic(
        Scene& scene,
        DeleteNodeMutex& delete_node_mutex,
        float speed = 1);
    ~MoveSceneLogic();

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_without_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    Scene& scene_;
    bool first_render_;
    DeleteNodeMutex& delete_node_mutex_;
    float speed_;
    std::chrono::steady_clock::time_point last_time_;
};

}
