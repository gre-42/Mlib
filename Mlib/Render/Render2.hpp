#pragma once
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Scene_Graph/Camera_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <mutex>

struct GLFWwindow;

namespace Mlib {

struct RenderResults;
class SceneNodeResources;
class RenderingResources;
class RenderLogic;
class Scene;
class Window;
class ButtonStates;

template <class TData, size_t n>
class TransformationMatrix;

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
        const SceneGraphConfig& scene_graph_config = SceneGraphConfig{},
        ButtonStates* button_states = nullptr);

    void operator () (
        RenderLogic& logic,
        std::recursive_mutex& mutex,
        const SceneGraphConfig& scene_graph_config = SceneGraphConfig{},
        ButtonStates* button_states = nullptr);

    void operator () (
        const Scene& scene,
        bool rotate = false,
        float scale = 1,
        const SceneGraphConfig& scene_graph_config = SceneGraphConfig{});

    void render_depth_map(
        const Array<float>& rgb_picture,
        const Array<float>& depth_picture,
        const FixedArray<float, 3, 3>& intrinsic_matrix,
        bool rotate = false,
        float scale = 1,
        const SceneGraphConfig& scene_graph_config = SceneGraphConfig{},
        const CameraConfig& camera_config = CameraConfig{});

    void render_height_map(
        const Array<float>& rgb_picture,
        const Array<float>& height_picture,
        const TransformationMatrix<float, 2>& normalization_matrix,
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
