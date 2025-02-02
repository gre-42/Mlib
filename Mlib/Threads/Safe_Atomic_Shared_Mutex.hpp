#pragma once
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <cstdint>
#include <mutex>

namespace Mlib {

class SafeAtomicSharedMutex {
    SafeAtomicSharedMutex(const SafeAtomicSharedMutex&) = delete;
    SafeAtomicSharedMutex& operator = (const SafeAtomicSharedMutex&) = delete;
public:
    inline SafeAtomicSharedMutex()
        : locked_{ false }
        , nlocked_shared_{ 0 }
    {}
    inline ~SafeAtomicSharedMutex() = default;
    inline void lock() {
        while (!try_lock());
    }
    inline bool try_lock() {
        if (!mutex_.try_lock()) {
            return false;
        }
        if (locked_ || (nlocked_shared_ != 0)) {
            mutex_.unlock();
            return false;
        }
        locked_ = true;
        mutex_.unlock();
        return true;
    }
    inline void unlock() {
        std::scoped_lock lock{ mutex_ };
        locked_ = false;
    }
    void lock_shared() {
        while (!try_lock_shared());
    }
    bool try_lock_shared() {
        std::scoped_lock lock{ mutex_ };
        if (locked_) {
            return false;
        }
        ++nlocked_shared_;
        return true;
    }
    void unlock_shared() {
        std::scoped_lock lock{ mutex_ };
        --nlocked_shared_;
    }
private:
    bool locked_;
    uint32_t nlocked_shared_;
    AtomicMutex mutex_;
};

}
