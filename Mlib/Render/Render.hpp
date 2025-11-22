#pragma once
#ifndef __ANDROID__

#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <atomic>
#include <chrono>
#include <iosfwd>
#include <memory>
#include <vector>

struct GLFWwindow;

namespace Mlib {

class IWindow;
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
struct InputConfig;
class SetFps;

template <typename TData, size_t... tshape>
class FixedArray;

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class Render {
public:
    explicit Render(
        const RenderConfig& render_config,
        const InputConfig& input_config,
        std::atomic_size_t& num_renderings,
        SetFps& set_fps,
        std::function<std::chrono::steady_clock::time_point()> frame_time,
        RenderResults* render_results = nullptr);
    ~Render();

    void print_hardware_info(std::ostream& ostr) const;

    Renderer generate_renderer() const;

    void render(
        RenderLogic& logic,
        const std::function<void()>& event_handler,
        const SceneGraphConfig& scene_graph_config = SceneGraphConfig(),
        std::function<void(uint32_t)>* char_callback = nullptr,
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
        const std::vector<TransformationMatrix<float, ScenePos, 3>>* beacon_locations = nullptr) const;

    void render_node(
        std::unique_ptr<SceneNode>&& node,
        const FixedArray<float, 3>& background_color,
        bool rotate,
        float scale,
        float camera_z,
        const SceneGraphConfig& scene_graph_config,
        std::unique_ptr<Camera>&& camera,
        const std::vector<TransformationMatrix<float, ScenePos, 3>>* beacon_locations = nullptr) const;

    GLFWwindow& glfw_window() const;
    IWindow& window() const;

    void request_window_close();
    bool window_should_close() const;

private:
    std::atomic_size_t& num_renderings_;
    SetFps& set_fps_;
    std::function<std::chrono::steady_clock::time_point()> frame_time_;
    RenderResults* render_results_;
    const RenderConfig& render_config_;
    const InputConfig& input_config_;
    std::unique_ptr<Window> window_;
};

}

#endif
