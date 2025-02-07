#pragma once
#include <Mlib/Threads/J_Thread.hpp>
#include <functional>
#include <future>
#include <list>

namespace Mlib {

class LaunchAsync {
public:
    explicit LaunchAsync(const std::string& thread_name);
    ~LaunchAsync();
    std::future<void> operator () (std::function<void()> task);
private:
    std::mutex mutex_;
    std::condition_variable task_ready_cv_;
    std::list<std::pair<std::promise<void>, std::function<void()>>> tasks_;
    JThread thread_;
};

}
