#pragma once
#include <cstddef>
#include <exception>

namespace Mlib {

struct RenderResults;
class RenderLogic;
class Window;
class ButtonStates;
class CursorStates;
struct SceneGraphConfig;
struct RenderConfig;
class EventHandler;

class Renderer {
    friend EventHandler;
public:
    Renderer(
        Window& window,
        const RenderConfig& render_config,
        size_t& num_renderings,
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
    size_t& num_renderings_;
    RenderResults* render_results_;
};

class EventHandler {
public:
    EventHandler(
        Renderer& renderer,
        ButtonStates* button_states,
        CursorStates* cursor_states,
        CursorStates* scroll_wheel_states);
    ~EventHandler();
private:
    Renderer& renderer_;
    ButtonStates* button_states_;
    CursorStates* cursor_states_;
    CursorStates* scroll_wheel_states_;
};

}
