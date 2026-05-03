#pragma once
#include <functional>

namespace Mlib {

void set_animation_frame_thread();
void exit_with_live_runtime();
// Execute func periodically using
// emscripten_request_animation_frame_loop,
// until "exit_loop() == true".
void set_animation_frame_func_loop(
    const std::function<void()>& func,
    const std::function<bool()>& exit_loop);
// Execute func once in animation thread, using
// emscripten_set_main_loop_arg, and wait until
// execution is exit_loop.
void execute_in_animation_frame_thread(
    const std::function<void()>& func);
void execute_in_main_thread(
    const std::function<void()>& func);

}
