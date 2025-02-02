#pragma once
#include <atomic>
#include <condition_variable>
#include <mutex>

namespace Mlib {

class Dispatcher {
    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator = (const Dispatcher&) = delete;
public:
    explicit Dispatcher(std::chrono::milliseconds wait_time);
    ~Dispatcher();
    void produce();
    void wait_for_data();
    void consume();
private:
    unsigned int nparticipants_;
    std::atomic_uint nwaiting_;
    unsigned int nproduced_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::chrono::milliseconds wait_time_;
};

}
