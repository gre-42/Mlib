#pragma once
#include <Mlib/Threads/Worker_Status.hpp>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace Mlib {

class BackgroundLoop {
public:
    BackgroundLoop();
    ~BackgroundLoop();
    WorkerStatus tick(size_t update_interval);
    void run(const std::function<void()>& task);
    bool done() const;
private:
    std::thread thread_;
    std::function<void()> task_;
    size_t i_ = 0;
    std::atomic_bool done_ = true;
    std::atomic_bool shutdown_requested_ = false;
    std::condition_variable task_ready_cv_;
    std::mutex mutex_;
};

}
