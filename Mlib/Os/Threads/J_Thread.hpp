#pragma once
#include <cstdint>
#include <functional>
#include <thread>

namespace Mlib {

void set_thread_limit(uint32_t nthreads);

class StopToken {
public:
    StopToken();
    void request_stop();
    bool stop_requested() const;
private:
    std::atomic_bool stop_requested_;
};

class JThread {
public:
    explicit JThread(std::function<void()> f);
    explicit JThread(std::function<void(const StopToken& stop_token)> f);
    ~JThread();
    void request_stop();
    StopToken& get_stop_token();
    const StopToken& get_stop_token() const;
    bool joinable() const;
    void join();
private:
    void check_and_increase_thread_count() const;
    StopToken stop_token_;
    std::thread thread_;
};

}
