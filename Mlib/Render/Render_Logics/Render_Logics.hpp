#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <list>
#include <memory>

namespace Mlib {

class SceneNode;

class RenderLogics: public RenderLogic, public DestructionObserver {
public:
    ~RenderLogics();
    virtual void initialize(GLFWwindow* window) override;
    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<float, 4, 4>& vp() const override;
    virtual bool requires_postprocessing() const override;

    virtual void notify_destroyed(void* destroyed_object) override;

    void prepend(SceneNode* scene_node, const std::shared_ptr<RenderLogic>& render_logic);
    void append(SceneNode* scene_node, const std::shared_ptr<RenderLogic>& render_logic);
private:
    void insert(SceneNode* scene_node, const std::shared_ptr<RenderLogic>& render_logic, bool prepend);
    GLFWwindow* window_ = nullptr;
    std::list<std::pair<SceneNode*, std::shared_ptr<RenderLogic>>> render_logics_;
    std::list<std::pair<SceneNode*, std::shared_ptr<RenderLogic>>> to_be_initialized_;
};

}
