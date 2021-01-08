#include "Background_Loop.hpp"

using namespace Mlib;

BackgroundLoop::BackgroundLoop() {
    task_ready_mutex_.lock();
    thread_ = std::thread{[this](){
        while (true) {
            task_ready_mutex_.lock();
            if (shutdown_requested_) {
                return;
            }
            task_();
            done_ = true;
        }
    }};
}

BackgroundLoop::~BackgroundLoop() {
    shutdown_requested_ = true;
    task_ready_mutex_.unlock();
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
    task_ = task;
    done_ = false;
    task_ready_mutex_.unlock();
}

bool BackgroundLoop::done() const {
    return done_;
}
