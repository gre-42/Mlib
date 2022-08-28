#pragma once
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <memory>
#include <vector>

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
class CursorStates;
class Camera;
class Renderer;
struct RenderConfig;

template <typename TData, size_t... tshape>
class FixedArray;

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class Render2 {
public:
    explicit Render2(
        const RenderConfig& render_config,
        size_t& num_renderings,
        RenderResults* render_results = nullptr);
    ~Render2();

    void print_hardware_info() const;

    Renderer generate_renderer() const;

    void render(
        RenderLogic& logic,
        const SceneGraphConfig& scene_graph_config = SceneGraphConfig(),
        ButtonStates* button_states = nullptr,
        CursorStates* cursor_states = nullptr,
        CursorStates* scroll_wheel_states = nullptr) const;

    void render_scene(
        const Scene& scene,
        const FixedArray<float, 3>& background_color,
        bool rotate = false,
        float scale = 1,
        float camera_z = 0,
        const SceneGraphConfig& scene_graph_config = SceneGraphConfig(),
        const std::vector<TransformationMatrix<float, double, 3>>* beacon_locations = nullptr) const;

    void render_node(
        std::unique_ptr<SceneNode>&& node,
        const FixedArray<float, 3>& background_color,
        bool rotate,
        float scale,
        float camera_z,
        const SceneGraphConfig& scene_graph_config,
        std::unique_ptr<Camera>&& camera,
        const std::vector<TransformationMatrix<float, double, 3>>* beacon_locations = nullptr) const;
    
    GLFWwindow* window() const;

    bool window_should_close() const;

private:
    size_t& num_renderings_;
    RenderResults* render_results_;
    const RenderConfig& render_config_;
    std::unique_ptr<Window> window_;
};

}
