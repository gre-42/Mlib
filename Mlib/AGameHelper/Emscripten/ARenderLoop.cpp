#include "ARenderLoop.hpp"
#include <Mlib/AGameHelper/Emscripten/AEngine.hpp>
#include <Mlib/AGameHelper/Emscripten/Execute_Func_On_Main_Thread.hpp>
#include <Mlib/OpenGL/IRenderer.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <Mlib/Threads/Thread_Local.hpp>
#include <GLES3/gl3.h>
#include <emscripten/html5.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>
#include <mutex>
#include <thread>

using namespace Mlib;

static bool& main_loop_valid() {
    THREAD_LOCAL(bool) result = false;
    return result;
}

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
    if (!main_loop_valid()) {
        return;
    }
    auto& re = *static_cast<ARenderLoopAndExitLoop*>(arg);
    try {
        if (re.exit_loop()) {
            main_loop_valid() = false;
            emscripten_cancel_main_loop();
            {
                std::lock_guard<std::mutex> lock(re.mutex);
                re.finished = true;
            }
            re.cv.notify_one();
            return;
        }
    } catch (const std::exception& e) {
        verbose_abort("Unhandled exception in render loop: " + std::string{e.what()});
    } catch (...) {
        verbose_abort("Unknown unhandled exception in render loop");
    }
    try {
        re.loop.aengine_.draw_frame(Mlib::RenderEvent::LOOP);
    } catch (...) {
        add_unhandled_exception(std::current_exception());
    }
}

void start_loop_proxy(ARenderLoopAndExitLoop& re) {
    try {
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
    } catch (...) {
        add_unhandled_exception(std::current_exception());
        return;
    }
    emscripten_set_main_loop_arg(run_main_loop_iteration, &re, 0, false);
    main_loop_valid() = true;
}

void ARenderLoop::render_loop(const std::function<bool()>& exit_loop) {
    // while (!exit_loop()) {
    //     aengine_.draw_frame(Mlib::RenderEvent::LOOP);
    //     emscripten_current_thread_process_queued_calls();
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1));
    // }
    ARenderLoopAndExitLoop re{*this, exit_loop};
    execute_func_on_main_thread([&re](){ start_loop_proxy(re); });
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
