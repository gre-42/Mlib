#include "Background_Task.hpp"
#include <stdexcept>

using namespace Mlib;

BackgroundTask::~BackgroundTask() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

WorkerStatus BackgroundTask::tick(size_t update_interval) {
    i_ = (i_ + 1) % update_interval;
    if (done_) {
        if (thread_.joinable()) {
            thread_.join();
        }
        if (i_ == 0) {
            return WorkerStatus::IDLE;
        } else {
            return WorkerStatus::BUSY;
        }
    } else {
        return WorkerStatus::BUSY;
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

bool BackgroundTask::done() const {
    return done_;
}
