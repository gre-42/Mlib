#include "Background_Loop.hpp"
#include <Mlib/Threads/Set_Thread_Name.hpp>

using namespace Mlib;

BackgroundLoop::BackgroundLoop() {
    thread_ = std::thread{[this](){
        set_thread_name("Background loop");
        while (true) {
            std::unique_lock lck{ mutex_ };
            task_ready_cv_.wait(lck, [this]() { return !done_ || shutdown_requested_; });
            if (shutdown_requested_) {
                return;
            }
            task_();
            done_ = true;
        }
    }};
}

BackgroundLoop::~BackgroundLoop() {
    {
        std::unique_lock lock{ mutex_ };
        shutdown_requested_ = true;
    }
    task_ready_cv_.notify_one();
    thread_.join();
}

WorkerStatus BackgroundLoop::tick(size_t update_interval) {
    i_ = (i_ + 1) % update_interval;
    if (done_) {
        if (i_ == 0) {
            return WorkerStatus::IDLE;
        } else {
            return WorkerStatus::BUSY;
        }
    } else {
        return WorkerStatus::BUSY;
    }
}

void BackgroundLoop::run(const std::function<void()>& task) {
    if (!thread_.joinable()) {
        throw std::runtime_error("BackgroundLoop::run despite not joinable thread");
    }
    if (!done_) {
        throw std::runtime_error("BackgroundLoop::run despite not done");
    }
    std::unique_lock lck{ mutex_ };
    task_ = task;
    done_ = false;
    task_ready_cv_.notify_one();
}

bool BackgroundLoop::done() const {
    return done_;
}
