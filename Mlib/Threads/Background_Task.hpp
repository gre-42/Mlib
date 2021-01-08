#pragma once
#include <Mlib/Threads/Worker_Status.hpp>
#include <functional>
#include <thread>

namespace Mlib {

class BackgroundTask {
public:
    ~BackgroundTask();
    WorkerStatus tick(size_t update_interval);
    void run(const std::function<void()>& task);
    bool done() const;
private:
    std::thread thread_;
    std::function<void()> task_;
    size_t i_ = 0;
    std::atomic_bool done_ = true;
};

}
