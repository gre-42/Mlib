
#include "Gl_Context_Guard.hpp"
#include <Mlib/OpenGL/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/OpenGL/IWindow.hpp>
#include <thread>

static std::thread::id render_thread_id = std::thread::id();

using namespace Mlib;

GlContextGuard::GlContextGuard(const IWindow& window)
    : window_{window}
{
    window_.make_current();
    render_thread_id = std::this_thread::get_id();
    execute_render_gc();
}

GlContextGuard::~GlContextGuard() {
    execute_render_gc();
    window_.unmake_current();
    render_thread_id = std::thread::id();
}

void GlContextGuard::assert_this_thread_is_renderer() {
    if (render_thread_id == std::thread::id()) {
        throw std::runtime_error("No render thread is set");
    }
    if (std::this_thread::get_id() != render_thread_id) {
        throw std::runtime_error("Thread is not the render thread");
    }
}
