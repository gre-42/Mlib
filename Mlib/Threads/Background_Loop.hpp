#pragma once
#include <Mlib/Threads/J_Thread.hpp>
#include <Mlib/Threads/Worker_Status.hpp>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace Mlib {

/**
 * Like BackgroundTask, but reuses the same thread for each task.
 */
class BackgroundLoop {
public:
    BackgroundLoop();
    ~BackgroundLoop();
    WorkerStatus tick(size_t update_interval);
    void run(const std::function<void()>& task);
    bool done() const;
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
