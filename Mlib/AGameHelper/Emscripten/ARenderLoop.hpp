#pragma once
#include <emscripten/html5.h>
#include <functional>

struct ARenderLoopAndExitLoop;
void run_main_loop_iteration(void* arg);
void start_loop_proxy(ARenderLoopAndExitLoop& arg);

namespace Mlib {

class AEngine;

class ARenderLoop {
    friend void ::run_main_loop_iteration(void* arg);
    friend void ::start_loop_proxy(ARenderLoopAndExitLoop& re);
public:
    explicit ARenderLoop(AEngine& aengine);
    ~ARenderLoop();

    void render_loop(const std::function<bool()>& exit_loop = [](){return false;});
    bool destroy_requested() const;
private:
    AEngine& aengine_;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx_;
};

}
