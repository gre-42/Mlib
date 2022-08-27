#pragma once
#include <cstddef>
#include <exception>

namespace Mlib {

struct RenderResults;
class RenderLogic;
class Window;
class ButtonStates;
struct SceneGraphConfig;
struct RenderConfig;

class Renderer {
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
    void handle_events(ButtonStates* button_states) const;
    void render_and_handle_events(
        RenderLogic& logic,
        const SceneGraphConfig& scene_graph_config,
        ButtonStates* button_states);
private:
    bool continue_rendering() const;
    Window& window_;
    const RenderConfig& render_config_;
    size_t& num_renderings_;
    RenderResults* render_results_;
};

}
