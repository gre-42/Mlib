#pragma once
#include <atomic>
#include <cstddef>
#include <exception>

#ifndef __ANDROID__

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

class Renderer {
    friend void handle_events(
        Renderer &renderer,
        ButtonStates *button_states,
        CursorStates *cursor_states,
        CursorStates *scroll_wheel_states);
public:
    Renderer(
        Window& window,
        const RenderConfig& render_config,
        std::atomic_size_t& num_renderings,
        SetFps& set_fps,
        RenderResults* render_results);
    ~Renderer();
    void render(
        RenderLogic& logic,
        const SceneGraphConfig& scene_graph_config) const;
    void render_and_handle_events(
        RenderLogic& logic,
        const SceneGraphConfig& scene_graph_config,
        ButtonStates* button_states,
        CursorStates* cursor_states,
        CursorStates* scroll_wheel_states);
private:
    bool continue_rendering() const;
    Window& window_;
    const RenderConfig& render_config_;
    std::atomic_size_t& num_renderings_;
    RenderResults* render_results_;
    SetFps& set_fps_;
};

void handle_events(
    Renderer& renderer,
    ButtonStates* button_states,
    CursorStates* cursor_states,
    CursorStates* scroll_wheel_states);

}

#endif
