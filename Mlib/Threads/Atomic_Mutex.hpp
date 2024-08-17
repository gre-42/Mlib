#pragma once
#include <atomic>

namespace Mlib {

class AtomicMutex {
    AtomicMutex(const AtomicMutex&) = delete;
    AtomicMutex& operator = (const AtomicMutex&) = delete;
public:
    inline AtomicMutex()
        : ctr_{ 0 }
    {}
    inline ~AtomicMutex() = default;
    inline void lock() {
        while (!try_lock());
    }
    inline bool try_lock() {
        ++ctr_;
        if (ctr_ == 1) {
            return true;
        } else {
            --ctr_;
            return false;
        }
    }
    inline void unlock() {
        --ctr_;
    }
private:
    std::atomic_uint32_t ctr_;
};

}
