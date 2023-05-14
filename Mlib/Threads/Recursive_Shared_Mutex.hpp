#pragma once
#include <shared_mutex>
#include <thread>

namespace Mlib {

class RecursiveSharedMutex: private std::shared_mutex {
public:
    RecursiveSharedMutex()
    : count_{0}
    {}
    ~RecursiveSharedMutex()
    {}
    void lock() {
        if (!is_owner()) {
            std::shared_mutex::lock();
            owner_ = std::this_thread::get_id();
        }
        ++count_;
    }
    void unlock() {
        --count_;
        if (count_ == 0) {
            owner_ = std::thread::id();
            std::shared_mutex::unlock();
        }
    }
    void lock_shared() {
        if (!is_owner()) {
            std::shared_mutex::lock_shared();
        }
    }
    void unlock_shared() {
        if (!is_owner()) {
            std::shared_mutex::unlock_shared();
        }
    }
protected:
    bool is_owner() const {
        return owner_ == std::this_thread::get_id();
    }
private:
    std::atomic<std::thread::id> owner_;
    std::atomic_uint32_t count_;
};

}
