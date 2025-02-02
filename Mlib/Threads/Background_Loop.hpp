#pragma once
#include <Mlib/Threads/J_Thread.hpp>
#include <Mlib/Threads/Worker_Status.hpp>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace Mlib {

class BackgroundLoop {
public:
    explicit BackgroundLoop(std::string thread_name);
    ~BackgroundLoop();
    WorkerStatus tick(size_t update_interval);
    void run(const std::function<void()>& task);
    bool try_run(const std::function<void()>& task);
    bool done() const;
    void wait_until_done() const;
    void wait_until_done_and_run(const std::function<void()>& task);
    void shutdown();
private:
    size_t i_;
    bool done_;
    mutable std::mutex mutex_;
    std::condition_variable task_ready_cv_;
    mutable std::condition_variable done_cv_;
    JThread thread_;
    std::function<void()> task_;
};

}
