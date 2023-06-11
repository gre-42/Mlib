#pragma once
#include <shared_mutex>
#include <thread>

namespace Mlib {

class RecursiveSharedMutex {
public:
    RecursiveSharedMutex()
    : count_{0}
    {}
    ~RecursiveSharedMutex() = default;
    void lock() {
        if (!is_owner()) {
            mutex_.lock();
            owner_ = std::this_thread::get_id();
        }
        ++count_;
    }
    void unlock() {
        --count_;
        if (count_ == 0) {
            owner_ = std::thread::id();
            mutex_.unlock();
        }
    }
    void lock_shared() {
        if (!is_owner()) {
            mutex_.lock_shared();
        }
    }
    void unlock_shared() {
        if (!is_owner()) {
            mutex_.unlock_shared();
        }
    }
    bool is_owner() const {
        return owner_ == std::this_thread::get_id();
    }
private:
    std::shared_mutex mutex_;
    std::atomic<std::thread::id> owner_;
    std::atomic_uint32_t count_;
};

}
