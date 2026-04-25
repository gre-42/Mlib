#pragma once
#include <functional>
#include <list>

struct EmscriptenKeyboardEvent;

namespace Mlib {

class IRenderer;
class ButtonStates;
enum class RenderEvent;
struct LayoutConstraintParameters;

class AEngine {
public:
    explicit AEngine(
        IRenderer& renderer,
        ButtonStates& button_states);
    ~AEngine();
    void draw_frame(Mlib::RenderEvent event);
    LayoutConstraintParameters layout_parameters_x() const;
    LayoutConstraintParameters layout_parameters_y() const;
    void add_on_save_state(std::function<void()> func);
private:
    static bool key_callback(int event_type, const EmscriptenKeyboardEvent* e, void* user_data);
    IRenderer& renderer_;
    ButtonStates& button_states_;
    std::list<std::function<void()>> on_save_state_;
};

}
