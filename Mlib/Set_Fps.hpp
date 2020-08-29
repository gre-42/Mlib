#pragma once
#include <chrono>

namespace Mlib {

class SetFps {
public:
    SetFps();
    void tick(float dt, float max_residual_time, bool print_residual_time);
    void toggle_pause_resume();
    void pause();
    void resume();
    bool paused() const;
private:
    std::chrono::steady_clock::time_point sim_time_;
    bool paused_;
};

}
