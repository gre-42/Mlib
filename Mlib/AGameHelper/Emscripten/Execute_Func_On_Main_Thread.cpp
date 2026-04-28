#include "Execute_Func_On_Main_Thread.hpp"
#include <Mlib/Os/Os.hpp>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>

using namespace Mlib;

static void execute_func(void* arg) {
    const auto& func = *(const std::function<void()>*)arg;
    func();
}

void Mlib::execute_func_on_main_thread(const std::function<void()>& func) {
    try {
        em_proxying_queue* queue = emscripten_proxy_get_system_queue();
        pthread_t main_thread = emscripten_main_runtime_thread_id();
        emscripten_proxy_sync(
            queue,
            main_thread,
            execute_func,
            &const_cast<std::function<void()>&>(func)
        );
    } catch (const std::exception& e) {
        verbose_abort("Unhandled exception during execution on main thread: " + std::string{e.what()});
    } catch (...) {
        verbose_abort("Unknown unhandled exception during execution on main thread");
    }
}
