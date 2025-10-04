#pragma once

#ifndef __ANDROID__

#include <atomic>
#include <chrono>
#include <cstddef>
#include <exception>
#include <functional>

namespace Mlib {

class SetFps;
struct RenderResults;
class RenderLogic;
class Window;
class ButtonStates;
class CursorStates;
struct SceneGraphConfig;
struct RenderConfig;
template <typename TData, size_t... tshape>
class FixedArray;
struct InputConfig;

class Renderer {
    friend void handle_events(
        Renderer &renderer,
        std::function<void(uint32_t)>* char_callback,
        ButtonStates *button_states,
        CursorStates *cursor_states,
        CursorStates *scroll_wheel_states,
        const InputConfig& input_config,
        const std::function<void()>& callback);
public:
    Renderer(
        Window& window,
        const RenderConfig& render_config,
        const InputConfig& input_config,
        std::atomic_size_t& num_renderings,
        SetFps& set_fps,
        std::function<std::chrono::steady_clock::time_point()> frame_time,
        RenderResults* render_results);
    ~Renderer();
    void render(
        RenderLogic& logic,
        const SceneGraphConfig& scene_graph_config) const;
    void render_and_handle_events(
        RenderLogic& logic,
        const std::function<void()>& event_callback,
        const SceneGraphConfig& scene_graph_config,
        std::function<void(uint32_t)>* char_callback,
        ButtonStates* button_states,
        CursorStates* cursor_states,
        CursorStates* scroll_wheel_states);
private:
    bool continue_rendering() const;
    Window& window_;
    const RenderConfig& render_config_;
    const InputConfig& input_config_;
    std::atomic_size_t& num_renderings_;
    RenderResults* render_results_;
    SetFps& set_fps_;
    std::function<std::chrono::steady_clock::time_point()> frame_time_;
};

void handle_events(
    Renderer& renderer,
    std::function<void(uint32_t)>* char_callback,
    ButtonStates* button_states,
    CursorStates* cursor_states,
    CursorStates* scroll_wheel_states,
    const InputConfig& input_config,
    const std::function<void()>& callback);

}

#endif
