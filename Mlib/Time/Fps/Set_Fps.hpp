#pragma once
#include <atomic>
#include <chrono>
#include <functional>
#include <list>
#include <mutex>

namespace Mlib {

class ISleeper;

class SetFps {
public:
    explicit SetFps(
        ISleeper* sleeper,
        std::function<std::chrono::steady_clock::time_point()> simulated_time = std::function<std::chrono::steady_clock::time_point()>(),
        std::function<bool()> paused = std::function<bool()>(),
        std::function<void()> on_tick = std::function<void()>());
    ~SetFps();
    void tick(std::chrono::steady_clock::time_point completed_time);
    void execute_oldest_funcs();
    std::chrono::steady_clock::time_point completed_time() const;
    std::chrono::steady_clock::time_point simulated_time() const;
    bool paused() const;
    void execute(const std::function<void()>& func);
    void request_stop();
    bool is_up_to_date() const;
private:
    bool execute_oldest_func();
    std::list<std::function<void()>> funcs_;
    std::atomic_bool stop_requested_;
    std::atomic<std::chrono::steady_clock::time_point> completed_time_;
    std::function<std::chrono::steady_clock::time_point()> simulated_time_;
    std::function<bool()> paused_;
    std::function<void()> on_tick_;
    std::mutex execute_mutex_;
    ISleeper* sleeper_;
};

}
