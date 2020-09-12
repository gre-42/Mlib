#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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
        size_t& num_renderings,
        RenderResults* render_results = nullptr,
        const RenderConfig& render_config = RenderConfig{});
    ~Render2();

    void print_hardware_info() const;

    void operator () (
        RenderLogic& logic,
        std::shared_mutex& mutex,
        const SceneGraphConfig& scene_graph_config = SceneGraphConfig{});

    void operator () (
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
    
    GLFWwindow* window() const;

    bool window_should_close() const;

private:
    size_t& num_renderings_;
    RenderResults* render_results_;
    RenderConfig render_config_;
    std::unique_ptr<Window> window_;
};

}
