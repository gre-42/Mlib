#pragma once
#include <Mlib/Memory/Destruction_Guards.hpp>
#include <emscripten/html5_webgl.h>
#include <functional>
#include <list>

struct EmscriptenKeyboardEvent;

namespace Mlib {

class IRenderer;
class ButtonStates;
class CursorStates;
enum class RenderEvent;
struct LayoutConstraintParameters;

class AEngine {
public:
    explicit AEngine(
        IRenderer& renderer,
        ButtonStates& button_states,
        CursorStates& cursor_states,
        CursorStates& scroll_wheel_states);
    ~AEngine();
    void draw_frame(Mlib::RenderEvent event);
    LayoutConstraintParameters layout_parameters_x() const;
    LayoutConstraintParameters layout_parameters_y() const;
    void add_on_save_state(std::function<void()> func);
private:
    static EM_BOOL key_callback(int event_type, const EmscriptenKeyboardEvent* e, void* user_data);
    static EM_BOOL on_click(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
    static EM_BOOL on_mouse_move(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
    static EM_BOOL on_mouse_down(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
    static EM_BOOL on_mouse_up(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
    static EM_BOOL on_wheel_scroll(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData);
    DestructionGuards dgs_;
    IRenderer& renderer_;
    ButtonStates& button_states_;
    CursorStates& cursor_states_;
    CursorStates& scroll_wheel_states_;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx_;
    std::list<std::function<void()>> on_save_state_;
};

}
