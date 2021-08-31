#pragma once
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>

struct GLFWwindow;

namespace Mlib {

struct RenderResults;
class SceneNodeResources;
class RenderingResources;
class RenderLogic;
class Scene;
class SceneNode;
class Window;
class ButtonStates;
class Camera;

template <class TData, size_t n>
class TransformationMatrix;

class Render2 {
public:
    explicit Render2(
        const RenderConfig& render_config,
        size_t& num_renderings,
        RenderResults* render_results = nullptr);
    ~Render2();

    void print_hardware_info() const;

    void operator () (
        RenderLogic& logic,
        const SceneGraphConfig& scene_graph_config = SceneGraphConfig(),
        ButtonStates* button_states = nullptr);

    void operator () (
        const Scene& scene,
        bool rotate = false,
        float scale = 1,
        float camera_z = 0,
        const SceneGraphConfig& scene_graph_config = SceneGraphConfig(),
        const std::vector<TransformationMatrix<float, 3>>* beacon_locations = nullptr);

    void render_node(
        std::unique_ptr<SceneNode>&& node,
        bool rotate,
        float scale,
        float camera_z,
        const SceneGraphConfig& scene_graph_config,
        std::unique_ptr<Camera>&& camera,
        const std::vector<TransformationMatrix<float, 3>>* beacon_locations = nullptr);
    
    GLFWwindow* window() const;

    bool window_should_close() const;

private:
    size_t& num_renderings_;
    RenderResults* render_results_;
    const RenderConfig& render_config_;
    std::unique_ptr<Window> window_;
};

}
