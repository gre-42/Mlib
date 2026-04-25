#include "ARenderLoop.hpp"
#include <Mlib/AGameHelper/Emscripten/AEngine.hpp>
#include <Mlib/OpenGL/IRenderer.hpp>
#include <GLES3/gl3.h>
#include <emscripten/html5.h>

using namespace Mlib;

ARenderLoop::ARenderLoop(AEngine& aengine)
    :aengine_{aengine}
{
    // Init GLES context using HTML5 API
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2;
    
    // "#canvas" is the default ID used by Emscripten's shell
    auto ctx = emscripten_webgl_create_context("#canvas", &attrs);
    emscripten_webgl_make_context_current(ctx);
}

ARenderLoop::~ARenderLoop() = default;

void ARenderLoop::render_loop(const std::function<bool()>& exit_loop) {
    while (!exit_loop()) {
        aengine_.draw_frame(Mlib::RenderEvent::LOOP);
        // Yield (crucial for proxied threads)
        // This allows the browser to pump the proxy message queue,
        // and throttles the frame rate.
        emscripten_sleep(1); 
    }
}

bool ARenderLoop::destroy_requested() const {
    return false;
}
