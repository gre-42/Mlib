#include "Launch_Async.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Thread_Affinity.hpp>
#include <Mlib/Threads/Thread_Initializer.hpp>

using namespace Mlib;

LaunchAsync::LaunchAsync(const std::string& thread_name)
    : thread_ { [&](){
        ThreadInitializer init{ thread_name, ThreadAffinity::POOL };
        while (!thread_.get_stop_token().stop_requested()) {
            std::list<std::pair<std::promise<void>, std::function<void()>>> tasks;
            {
                std::unique_lock lck{ mutex_ };
                task_ready_cv_.wait(lck, [this]() { return !tasks_.empty() || thread_.get_stop_token().stop_requested(); });
                tasks = std::move(tasks_);
            }
            for (auto& [promise, task] : tasks) {
                if (!task) {
                    verbose_abort("Task is empty");
                }
                task();
                promise.set_value();
            }
        }
        }
    }
{}

LaunchAsync::~LaunchAsync() {
    thread_.request_stop();
    task_ready_cv_.notify_one();
}

std::future<void> LaunchAsync::operator() (std::function<void()> func) {
    auto promise = std::promise<void>();
    auto future = promise.get_future();
    std::scoped_lock lock{ mutex_ };
    tasks_.emplace_back(std::move(promise), std::move(func));
    task_ready_cv_.notify_one();
    return future;
}
