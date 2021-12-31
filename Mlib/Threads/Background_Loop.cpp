#include "Background_Loop.hpp"
#include <Mlib/Threads/Set_Thread_Name.hpp>

using namespace Mlib;

BackgroundLoop::BackgroundLoop()
: thread_{std::jthread{[this](){
        set_thread_name("Background loop");
        while (true) {
            std::unique_lock lck{ mutex_ };
            task_ready_cv_.wait(lck, [this]() { return !done_ || thread_.get_stop_token().stop_requested(); });
            if (thread_.get_stop_token().stop_requested()) {
                return;
            }
            task_();
            done_ = true;
        }
    }}}
{}

BackgroundLoop::~BackgroundLoop() {
    shutdown();
}

void BackgroundLoop::shutdown() {
    if (thread_.get_stop_token().stop_requested()) {
        return;
    }
    {
        std::lock_guard lock{ mutex_ };
        thread_.request_stop();
    }
    task_ready_cv_.notify_one();
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
    std::lock_guard lck{ mutex_ };
    task_ = task;
    done_ = false;
    task_ready_cv_.notify_one();
}

bool BackgroundLoop::done() const {
    return done_;
}
