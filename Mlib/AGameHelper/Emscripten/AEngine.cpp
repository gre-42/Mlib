#include "AEngine.hpp"
#include <Mlib/AGameHelper/Emscripten/AAnimation_Frame_Worker.hpp>
#include <Mlib/AGameHelper/Emscripten/AEmscripten_Result_To_String.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/OpenGL/IRenderer.hpp>
#include <Mlib/OpenGL/Input_Map/Key_Events.hpp>
#include <Mlib/OpenGL/Input_Map/Key_Map_I18n.hpp>
#include <Mlib/OpenGL/Ui/Button_States.hpp>
#include <Mlib/OpenGL/Ui/Cursor_States.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <emscripten/html5.h>
#include <stdexcept>

using namespace Mlib;

// static EM_BOOL on_window_resize(int eventType, const EmscriptenUiEvent *e, void *userData) {
//     int new_width = e->windowInnerWidth;
//     int new_height = e->windowInnerHeight;
//     emscripten_set_canvas_element_size("#canvas", new_width, new_height);
//     return EM_TRUE;
// }

void update_canvas_size() {
    double css_width, css_height;
    emscripten_get_element_css_size("#canvas", &css_width, &css_height);

    int canvas_width, canvas_height;
    emscripten_get_canvas_element_size("#canvas", &canvas_width, &canvas_height);

    if (canvas_width != (int)css_width || canvas_height != (int)css_height) {
        emscripten_set_canvas_element_size("#canvas", (int)css_width, (int)css_height);
    }
}

AEngine::AEngine(
    IRenderer& renderer,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states)
    : renderer_{renderer}
    , button_states_{button_states}
    , cursor_states_{cursor_states}
    , scroll_wheel_states_{scroll_wheel_states}
    , ctx_{0}
{
    {
        auto dgs_add = [this](std::function<void()> f){
            dgs_.add([f=std::move(f)](){ execute_in_main_thread(f); });
        };
        execute_in_main_thread([this, &dgs_add](){
            linfo() << "Register input callbacks";
            // Key down
            if (emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE, key_callback) != EMSCRIPTEN_RESULT_SUCCESS) {
                throw std::runtime_error("Could not set emscripten keydown callback");
            }
            dgs_add([](){emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, nullptr);});
            // Key up
            if (emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE, key_callback) != EMSCRIPTEN_RESULT_SUCCESS) {
                throw std::runtime_error("Could not set emscripten keyup callback");
            }
            dgs_add([](){emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, nullptr);});
            // Mouse click
            if (emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE, on_click) != EMSCRIPTEN_RESULT_SUCCESS) {
                throw std::runtime_error("Could not set emscripten click callback");
            }
            dgs_add([](){emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, nullptr);});
            // Mouse move
            if (emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE, on_mouse_move) != EMSCRIPTEN_RESULT_SUCCESS) {
                throw std::runtime_error("Could not set emscripten mouse move callback");
            }
            dgs_add([](){emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, nullptr);});
            // Mouse down
            if (emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE, on_mouse_down) != EMSCRIPTEN_RESULT_SUCCESS) {
                throw std::runtime_error("Could not set emscripten mouse down callback");
            }
            dgs_add([](){emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, nullptr);});
            // Mouse up
            if (emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE, on_mouse_up) != EMSCRIPTEN_RESULT_SUCCESS) {
                throw std::runtime_error("Could not set emscripten mouse up callback");
            }
            dgs_add([](){emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, nullptr);});
            // Scroll wheel
            if (emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE, on_wheel_scroll) != EMSCRIPTEN_RESULT_SUCCESS) {
                throw std::runtime_error("Could not set emscripten mouse up callback");
            }
            dgs_add([](){emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, nullptr);});
        });
    }
    {
        auto dgs_add = [this](std::function<void()> f){
            dgs_.add([f=std::move(f)](){ execute_in_animation_frame_thread(f); });
        };
        execute_in_animation_frame_thread([this, &dgs_add](){
            // if (emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, on_window_resize) != EMSCRIPTEN_RESULT_SUCCESS) {
            //     throw std::runtime_error("Could not register resize callback");
            // }
            // dgs_add([](){emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, nullptr);});
            // Init GLES context using HTML5 API
            EmscriptenWebGLContextAttributes attrs;
            emscripten_webgl_init_context_attributes(&attrs);
            attrs.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_DISALLOW;
            attrs.majorVersion = 2;

            // "#canvas" is the default ID used by Emscripten's shell
            ctx_ = emscripten_webgl_create_context("#canvas", &attrs);
            if (ctx_ <= 0) {
                throw std::runtime_error("WebGL context was not created. Is the selector '#canvas' in the DOM?");
            }
            {
                auto res = emscripten_webgl_make_context_current(ctx_);
                if (res != EMSCRIPTEN_RESULT_SUCCESS) {
                    throw std::runtime_error("Could not make context current (1): " + std::string(emscripten_result_to_str(res)));
                }
            }
            renderer_.load_resources();
            dgs_add([this](){ renderer_.unload_resources(); });
        });
    }
}

AEngine::~AEngine() = default;

void AEngine::draw_frame(Mlib::RenderEvent event) {
    update_canvas_size();
    auto lx = layout_parameters_x();
    auto ly = layout_parameters_y();
    renderer_.render(event, lx, ly);
}

LayoutConstraintParameters AEngine::layout_parameters_x() const {
    int width, height;
    if (emscripten_webgl_get_drawing_buffer_size(ctx_, &width, &height) != EMSCRIPTEN_RESULT_SUCCESS) {
        throw std::runtime_error("Could not determine canvas size");
    }
    double dpi = emscripten_get_device_pixel_ratio() * 96 * 2; // The factor 2 was determined by experiment.
    return LayoutConstraintParameters{
        .dpi = (float)dpi,
        .min_pixel = 0.f,
        .end_pixel = (float)width};
}

LayoutConstraintParameters AEngine::layout_parameters_y() const {
    int width, height;
    if (emscripten_webgl_get_drawing_buffer_size(ctx_, &width, &height) != EMSCRIPTEN_RESULT_SUCCESS) {
        throw std::runtime_error("Could not determine canvas size");
    }
    double dpi = emscripten_get_device_pixel_ratio() * 96 * 2; // The factor 2 was determined by experiment.
    return LayoutConstraintParameters{
        .dpi = (float)dpi,
        .min_pixel = 0.f,
        .end_pixel = (float)height};
}

EM_BOOL AEngine::key_callback(int event_type, const EmscriptenKeyboardEvent* e, void* user_data) {
    auto* engine = static_cast<AEngine*>(user_data);
    try {
        auto code = keys_map_i18n.get(VariableAndHash<std::string>{e->code});
        if ((code == DOM_VK_F11) || (code == DOM_VK_F12)) {
            return EM_FALSE;
        }
        engine->button_states_.notify_key_event(code, event_type);
    } catch (...) {
        add_unhandled_exception(std::current_exception());
    }
    return EM_TRUE;
}

void AEngine::add_on_save_state(std::function<void()> func) {
    on_save_state_.emplace_back(std::move(func));
}

EM_BOOL AEngine::on_click(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
    try {
        // This must be called inside a click or mousedown callback
        bool deferUntilInEventHandler = true;
        if (emscripten_request_pointerlock("#canvas", deferUntilInEventHandler) != EMSCRIPTEN_RESULT_SUCCESS) {
            throw std::runtime_error("Could not request pointerlock");
        }
    } catch (...) {
        add_unhandled_exception(std::current_exception());
    }
    return EM_TRUE;
}

EM_BOOL AEngine::on_mouse_move(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
    int dx = mouseEvent->movementX;
    int dy = mouseEvent->movementY;

    auto* engine = (AEngine*)userData;
    try {
        engine->cursor_states_.update_cursor(dx, dy);
    } catch (...) {
        add_unhandled_exception(std::current_exception());
    }
    return EM_TRUE;
}

EM_BOOL AEngine::on_mouse_down(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
    auto* engine = (AEngine*)userData;
    try {
        engine->button_states_.notify_mouse_button_event(mouseEvent->button, KEY_PRESS);
    } catch (...) {
        add_unhandled_exception(std::current_exception());
    }
    return EM_TRUE;
}

EM_BOOL AEngine::on_mouse_up(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
    auto* engine = (AEngine*)userData;
    try {
        engine->button_states_.notify_mouse_button_event(mouseEvent->button, KEY_RELEASE);
    } catch (...) {
        add_unhandled_exception(std::current_exception());
    }
    return EM_TRUE;
}

EM_BOOL AEngine::on_wheel_scroll(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData) {
    double dx = wheelEvent->deltaX;
    double dy = wheelEvent->deltaY;

    auto* engine = (AEngine*)userData;
    try {
        engine->scroll_wheel_states_.update_cursor(dx, dy);
    } catch (...) {
        add_unhandled_exception(std::current_exception());
    }
    return EM_TRUE;
}
