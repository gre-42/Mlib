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
        ISleeper& sleeper,
        const std::function<bool()>& paused = [](){return false;});
    ~SetFps();
    void tick();
    void execute_oldest_funcs();
    bool paused() const;
    void execute(const std::function<void()>& func);
    void request_stop();
    bool is_up_to_date() const;
private:
    bool execute_oldest_func();
    std::list<std::function<void()>> funcs_;
    std::atomic_bool stop_requested_;
    std::function<bool()> paused_;
    std::mutex execute_mutex_;
    ISleeper& sleeper_;
};

}
