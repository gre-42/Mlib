#pragma once
#include <functional>
#include <thread>

namespace Mlib {

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
    ~JThread();
    void request_stop();
    StopToken& get_stop_token();
    const StopToken& get_stop_token() const;
    bool joinable() const;
    void join();
private:
    StopToken stop_token_;
    std::thread thread_;
};

}
