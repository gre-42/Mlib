#pragma once
#include <chrono>
#include <functional>
#include <list>
#include <string>

namespace Mlib {

class SetFps {
public:
    explicit SetFps(const std::string& prefix);
    void tick(float dt, float max_residual_time, bool print_residual_time);
    void toggle_pause_resume();
    void pause();
    void resume();
    bool paused() const;
    void execute(const std::function<void()>& func);
private:
    std::chrono::steady_clock::time_point sim_time_;
    bool paused_;
    std::string prefix_;
    std::list<std::function<void()>> funcs_;
};

}
