#include "Thread.hpp"

using namespace Mlib;

BackgroundTask::~BackgroundTask() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

BackgroundTaskStatus BackgroundTask::tick(size_t update_interval) {
    i_ = (i_ + 1) % update_interval;
    if (done_) {
        if (thread_.joinable()) {
            thread_.join();
        }
        if (i_ == 0) {
            return BackgroundTaskStatus::IDLE;
        } else {
            return BackgroundTaskStatus::BUSY;
        }
    } else {
        return BackgroundTaskStatus::BUSY;
    }
}

void BackgroundTask::run(const std::function<void()>& task) {
    if (thread_.joinable()) {
        throw std::runtime_error("BackgroundTask::run despite joinable thread");
    }
    if (!done_) {
        throw std::runtime_error("BackgroundTask::run despite not done");
    }
    done_ = false;
    thread_ = std::thread{[this, task](){
        task();
        done_ = true;
    }};
}
