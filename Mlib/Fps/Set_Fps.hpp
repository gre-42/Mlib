#pragma once
#include <chrono>
#include <functional>
#include <list>
#include <string>

namespace Mlib {

class SetFps {
public:
    explicit SetFps(
        const std::string& prefix,
        const std::function<bool()>& paused = [](){return false;});
    void tick(
        float dt,
        float max_residual_time,
        bool control_fps,
        bool print_residual_time);
    bool paused() const;
    void execute(const std::function<void()>& func);
    void request_stop();
private:
    std::chrono::steady_clock::time_point sim_time_;
    std::string prefix_;
    std::list<std::function<void()>> funcs_;
    bool stop_requested_;
    std::function<bool()> paused_;
};

}
