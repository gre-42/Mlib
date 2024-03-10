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
    void shutdown();
private:
    size_t i_;
    std::atomic_bool done_;
    std::condition_variable task_ready_cv_;
    std::mutex mutex_;
    JThread thread_;
    std::function<void()> task_;
};

}
