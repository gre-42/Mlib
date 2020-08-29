#pragma once
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Scene_Graph/Camera_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <shared_mutex>

namespace Mlib {

class SceneNodeResources;
class RenderLogic;
class RenderResults;
class Scene;
class Window;

class Render2 {
public:
    explicit Render2(
        bool& leave_render_loop,
        RenderResults* render_results = nullptr,
        const RenderConfig& render_config = RenderConfig{});
    ~Render2();

    void operator () (
        SceneNodeResources& scene_node_resources,
        const Scene& scene,
        RenderLogic& logic,
        std::shared_mutex& mutex,
        const SceneGraphConfig& scene_graph_config = SceneGraphConfig{});

    void operator () (
        SceneNodeResources& scene_node_resources,
        const Scene& scene,
        bool rotate = false,
        float scale = 1,
        const SceneGraphConfig& scene_graph_config = SceneGraphConfig{});

    void operator () (
        const Array<float>& rgb_picture,
        const Array<float>& depth_picture,
        const Array<float>& intrinsic_matrix,
        bool rotate = false,
        float scale = 1,
        const SceneGraphConfig& scene_graph_config = SceneGraphConfig{},
        const CameraConfig& camera_config = CameraConfig{});
    
    bool window_should_close() const;

private:
    bool& leave_render_loop_;
    RenderResults* render_results_;
    const RenderConfig& render_config_;
    std::unique_ptr<Window> window_;
};

}
