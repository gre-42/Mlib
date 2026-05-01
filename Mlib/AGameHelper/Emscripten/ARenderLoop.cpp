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
