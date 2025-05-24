#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene/Generic_Scenes.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <cstdint>
#include <list>
#include <string>

namespace Mlib {

class RenderableScenes: public GenericScenes<RenderableScene>, public RenderLogic {
    RenderableScenes(const RenderableScenes&) = delete;
    RenderableScenes& operator = (const RenderableScenes&) = delete;
public:
    RenderableScenes();
    ~RenderableScenes();

    // RenderLogic
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
    
    void add_fullscreen_scene(std::string scene);
    void add_tiled_scene(uint32_t, std::string scene);
private:
    mutable SafeAtomicSharedMutex mutex_;
    std::list<std::string> fullscreen_scenes_;
    std::map<uint32_t, std::list<std::string>> tiled_scenes_;
};

}
