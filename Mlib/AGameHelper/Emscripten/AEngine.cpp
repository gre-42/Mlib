#include "AEngine.hpp"
#include <Mlib/AGameHelper/Emscripten/Execute_Func_On_Main_Thread.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/OpenGL/IRenderer.hpp>
#include <Mlib/OpenGL/Ui/Button_States.hpp>
#include <emscripten/html5.h>
#include <stdexcept>

using namespace Mlib;

AEngine::AEngine(
    IRenderer& renderer,
    ButtonStates& button_states)
    : renderer_{renderer}
    , button_states_{button_states}
{
    execute_func_on_main_thread([this](){
        emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE, key_callback);
        emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE, key_callback);
    });
}

AEngine::~AEngine() {
    execute_func_on_main_thread([](){
        emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, nullptr);
        emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, nullptr);
    });
}

void AEngine::draw_frame(Mlib::RenderEvent event) {
    renderer_.render(event, layout_parameters_x(), layout_parameters_y());
}

LayoutConstraintParameters AEngine::layout_parameters_x() const {
    int css_w, css_h;
    if (emscripten_get_canvas_element_size("#canvas", &css_w, &css_h) != EMSCRIPTEN_RESULT_SUCCESS) {
        throw std::runtime_error("Could not determine canvas size");
    }
    double dpi = emscripten_get_device_pixel_ratio() * 96;
    return LayoutConstraintParameters{
        .dpi = (float)dpi,
        .min_pixel = 0.f,
        .end_pixel = (float)css_w};
}

LayoutConstraintParameters AEngine::layout_parameters_y() const {
    int css_w, css_h;
    if (emscripten_get_canvas_element_size("#canvas", &css_w, &css_h) != EMSCRIPTEN_RESULT_SUCCESS) {
        throw std::runtime_error("Could not determine canvas size");
    }
    double dpi = emscripten_get_device_pixel_ratio() * 96;
    return LayoutConstraintParameters{
        .dpi = (float)dpi,
        .min_pixel = 0.f,
        .end_pixel = (float)css_h};
}

bool AEngine::key_callback(int event_type, const EmscriptenKeyboardEvent* e, void* user_data) {
    auto* engine = static_cast<AEngine*>(user_data);
    engine->button_states_.notify_key_event(integral_cast<int>(e->keyCode), event_type);
    return true;
}

void AEngine::add_on_save_state(std::function<void()> func) {
    on_save_state_.emplace_back(std::move(func));
}
