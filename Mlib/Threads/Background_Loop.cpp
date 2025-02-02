#include "Background_Loop.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Thread_Affinity.hpp>
#include <Mlib/Threads/Thread_Initializer.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Sleep.hpp>

using namespace Mlib;

BackgroundLoop::BackgroundLoop(std::string thread_name)
    : i_{ SIZE_MAX }
    , done_{ true }
    , thread_{ [this, tn = std::move(thread_name)](){
        try {
            ThreadInitializer ti{ tn, ThreadAffinity::POOL };
            while (true) {
                {
                    std::unique_lock lck{ mutex_ };
                    task_ready_cv_.wait(lck, [this]() { return !done_ || thread_.get_stop_token().stop_requested(); });
                    if (thread_.get_stop_token().stop_requested()) {
                        task_ = std::function<void()>();
                        return;
                    }
                    if (!task_) {
                        verbose_abort("Task not set");
                    }
                }
                task_();
                {
                    std::scoped_lock lck{ mutex_ };
                    task_ = std::function<void()>();
                    done_ = true;
                }
                done_cv_.notify_one();
            }
        } catch (const std::exception& e) {
            verbose_abort("Unhandled exception in background-loop: " + std::string{e.what()});
        }
    } }
{}

BackgroundLoop::~BackgroundLoop() {
    shutdown();
}

void BackgroundLoop::shutdown() {
    if (thread_.get_stop_token().stop_requested()) {
        return;
    }
    thread_.request_stop();
    task_ready_cv_.notify_one();
    done_cv_.notify_all();
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
    if (!try_run(task)) {
        THROW_OR_ABORT("BackgroundLoop::run despite not done");
    }
}

bool BackgroundLoop::try_run(const std::function<void()>& task) {
    if (!thread_.joinable()) {
        THROW_OR_ABORT("BackgroundLoop::run despite not joinable thread");
    }
    {
        std::scoped_lock lck{ mutex_ };
        if (!done_) {
            return false;
        }
        task_ = task;
        done_ = false;
    }
    task_ready_cv_.notify_one();
    return true;
}

bool BackgroundLoop::done() const {
    std::scoped_lock lock{ mutex_ };
    return done_;
}

void BackgroundLoop::wait_until_done() const {
    std::unique_lock lck{ mutex_ };
    done_cv_.wait(lck, [this]() { return done_ || thread_.get_stop_token().stop_requested(); });
}

void BackgroundLoop::wait_until_done_and_run(
    const std::function<void()>& task)
{
    while (!thread_.get_stop_token().stop_requested() && !try_run(task)) {
        wait_until_done();
    }
}
