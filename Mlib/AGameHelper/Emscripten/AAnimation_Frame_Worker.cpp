#include "AAnimation_Frame_Worker.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <atomic>
#include <condition_variable>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/proxying.h>
#include <mutex>
#include <stdexcept>
#include <thread>

using namespace Mlib;

namespace g {
static std::optional<pthread_t> animation_thread_id;
}

namespace Loop {

namespace g {
static const std::function<void()>* func = nullptr;
static const std::function<bool()>* exit_loop = nullptr;
static FastMutex execution_mutex;
static FastMutex input_mutex;
static std::mutex done_mutex;
static std::condition_variable done_cv;
static bool done = false;
}

static EM_BOOL render_frame(double time, void* user_data) {
    const std::function<void()>* func = [](){
        std::scoped_lock lock{g::input_mutex};
        return g::func;
    }();
    if (func == nullptr) {
        return EM_TRUE;
    }
    (*func)();
    if ((*g::exit_loop)()) {
        g::func = nullptr;
        g::exit_loop = nullptr;
        {
            std::scoped_lock lock{g::done_mutex};
            g::done = true;
        }
        g::done_cv.notify_one();
    }
    return EM_TRUE;
}

}

void Mlib::set_animation_frame_thread() {
    if (::g::animation_thread_id.has_value()) {
        throw std::runtime_error("Animation frame thread already set");
    }
    ::g::animation_thread_id = pthread_self();
    emscripten_request_animation_frame_loop(Loop::render_frame, nullptr);
}

void Mlib::exit_with_live_runtime() {
    emscripten_exit_with_live_runtime();
}

void Mlib::set_animation_frame_func_loop(
    const std::function<void()>& func,
    const std::function<bool()>& exit_loop)
{
    if (!::g::animation_thread_id.has_value()) {
        throw std::runtime_error("set_animation_frame_thread not called");
    }
    std::scoped_lock execution_lock{Loop::g::execution_mutex};
    if (pthread_self() == *::g::animation_thread_id) {
        throw std::runtime_error("set_animation_frame_func_loop called from the animation thread");
    }
    {
        std::scoped_lock lock{Loop::g::input_mutex};
        if (Loop::g::func != nullptr) {
            verbose_abort("set_animation_frame_func_loop internal error");
        }
        Loop::g::done = false;
        Loop::g::func = &func;
        Loop::g::exit_loop = &exit_loop;
    }
    {
        std::unique_lock lk(Loop::g::done_mutex);
        Loop::g::done_cv.wait(lk, [&]{return Loop::g::done;});
    }
}

void Mlib::execute_in_animation_frame_thread(
    const std::function<void()>& func)
{
    if (!::g::animation_thread_id.has_value()) {
        throw std::runtime_error("set_animation_frame_thread not called");
    }
    if (pthread_self() == *::g::animation_thread_id) {
        throw std::runtime_error("execute_in_animation_frame_thread called from the animation thread");
    }
    std::exception_ptr eptr = nullptr;
    emscripten::ProxyingQueue queue;
    queue.proxySync(*::g::animation_thread_id, [&]() {
        try {
            func();
        } catch (...) {
            eptr = std::current_exception();
        }
    });
    if (eptr) {
        std::rethrow_exception(eptr);
    }
}
