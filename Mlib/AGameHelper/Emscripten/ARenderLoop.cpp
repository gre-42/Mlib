#include "ARenderLoop.hpp"
#include <Mlib/AGameHelper/Emscripten/AAnimation_Frame_Worker.hpp>
#include <Mlib/AGameHelper/Emscripten/AEngine.hpp>
#include <Mlib/OpenGL/IRenderer.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <Mlib/Threads/Thread_Local.hpp>
#include <emscripten/html5.h>

using namespace Mlib;

ARenderLoop::ARenderLoop(AEngine& aengine)
    : aengine_{aengine}
{}

ARenderLoop::~ARenderLoop() = default;

void ARenderLoop::render_loop(const std::function<bool()>& exit_loop) {
    execute_in_animation_frame_thread([](){
        try {
            // Init GLES context using HTML5 API
            EmscriptenWebGLContextAttributes attrs;
            emscripten_webgl_init_context_attributes(&attrs);
            attrs.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_DISALLOW;
            attrs.majorVersion = 2;

            // "#canvas" is the default ID used by Emscripten's shell
            auto ctx = emscripten_webgl_create_context("#canvas", &attrs);
            if (ctx <= 0) {
                throw std::runtime_error("WebGL context was not created. Is the selector '#canvas' in the DOM?");
            }
            {
                auto res = emscripten_webgl_make_context_current(ctx);
                if (res != EMSCRIPTEN_RESULT_SUCCESS) {
                    throw std::runtime_error("Could not make context current (1)");
                }
            }
        } catch (...) {
            add_unhandled_exception(std::current_exception());
            return;
        }
    });
    if (unhandled_exceptions_occured()) {
        return;
    }
    set_animation_frame_func_loop([this](){
        try {
            aengine_.draw_frame(Mlib::RenderEvent::LOOP);
        } catch (...) {
            add_unhandled_exception(std::current_exception());
        }
    }, exit_loop);
}

bool ARenderLoop::destroy_requested() const {
    return false;
}
