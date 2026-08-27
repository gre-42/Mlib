#pragma once
#include <Mlib/Os/Threads/Thread_Affinity.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>

namespace Mlib {

void set_thread_limit(uint32_t nthreads);

class StopToken {
public:
    StopToken();
    void request_stop();
    bool stop_requested() const;
private:
    std::shared_ptr<std::atomic_bool> stop_requested_;
};

class JThread {
    JThread(const JThread&) = delete;
    JThread& operator = (const JThread&) = delete;
    JThread& operator = (JThread&&) = delete;
public:
    explicit JThread(
        std::string name,
        ThreadAffinity affinity,
        std::function<void()> f);
    explicit JThread(
        std::string name,
        ThreadAffinity affinity,
        std::function<void(StopToken stop_token)> f);
    ~JThread();
    void request_stop();
    StopToken get_stop_token();
    const StopToken get_stop_token() const;
    bool joinable() const;
    void join();
private:
    uint32_t check_and_increase_thread_count() const;
    StopToken stop_token_;
    std::thread thread_;
    std::string name_;
    uint32_t thread_number_;
};

}
