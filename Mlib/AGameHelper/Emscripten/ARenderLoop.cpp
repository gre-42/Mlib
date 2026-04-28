#include "ARenderLoop.hpp"
#include <Mlib/AGameHelper/Emscripten/AEngine.hpp>
#include <Mlib/OpenGL/IRenderer.hpp>
#include <Mlib/Os/Os.hpp>
#include <GLES3/gl3.h>
#include <emscripten/html5.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>
#include <mutex>
#include <thread>

using namespace Mlib;

ARenderLoop::ARenderLoop(AEngine& aengine)
    : aengine_{aengine}
    , ctx_{0}
{}

ARenderLoop::~ARenderLoop() = default;

struct ARenderLoopAndExitLoop {
    ARenderLoop& loop;
    const std::function<bool()>& exit_loop;

    // Synchronization primitives
    std::mutex mutex;
    std::condition_variable cv;
    bool finished = false;
};

void run_main_loop_iteration(void* arg) {
    auto& re = *static_cast<ARenderLoopAndExitLoop*>(arg);
    if (re.exit_loop()) {
        emscripten_cancel_main_loop();
        {
            std::lock_guard<std::mutex> lock(re.mutex);
            re.finished = true;
        }
        re.cv.notify_one();
        return;
    }
    re.loop.aengine_.draw_frame(Mlib::RenderEvent::LOOP);
}

void start_loop_proxy(void* arg) {
    auto& re = *static_cast<ARenderLoopAndExitLoop*>(arg);
    if (re.loop.ctx_ == 0) {
        // Init GLES context using HTML5 API
        EmscriptenWebGLContextAttributes attrs;
        emscripten_webgl_init_context_attributes(&attrs);
        attrs.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_DISALLOW;
        attrs.majorVersion = 2;

        // "#canvas" is the default ID used by Emscripten's shell
        re.loop.ctx_ = emscripten_webgl_create_context("#canvas", &attrs);
        if (re.loop.ctx_ <= 0) {
            throw std::runtime_error("WebGL context was not created. Is the selector '#canvas' in the DOM?");
        }
    }
    {
        auto res = emscripten_webgl_make_context_current(re.loop.ctx_);
        if (res != EMSCRIPTEN_RESULT_SUCCESS) {
            throw std::runtime_error("Could not make context current (1)");
        }
    }
    emscripten_set_main_loop_arg(run_main_loop_iteration, arg, 0, false);
}

void ARenderLoop::render_loop(const std::function<bool()>& exit_loop) {
    // while (!exit_loop()) {
    //     aengine_.draw_frame(Mlib::RenderEvent::LOOP);
    //     emscripten_current_thread_process_queued_calls();
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1));
    // }
    ARenderLoopAndExitLoop re{*this, exit_loop};
    em_proxying_queue* queue = emscripten_proxy_get_system_queue();
    pthread_t main_thread = emscripten_main_runtime_thread_id();
    emscripten_proxy_sync(
        queue,
        main_thread,
        start_loop_proxy,
        &re
    );
    // std::unique_lock<std::mutex> lk(re.mutex);
    // re.cv.wait(lk, [&re]{ return re.finished; });
    while (true) {
        {
            std::lock_guard<std::mutex> lock(re.mutex);
            if (re.finished) break;
        }
        emscripten_current_thread_process_queued_calls();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

bool ARenderLoop::destroy_requested() const {
    return false;
}
