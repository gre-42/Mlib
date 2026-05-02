#pragma once
#include <Mlib/Time/Sleep.hpp>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace Mlib {

class AtomicMutex {
    AtomicMutex(const AtomicMutex&) = delete;
    AtomicMutex& operator = (const AtomicMutex&) = delete;
public:
    inline AtomicMutex() = default;
    inline ~AtomicMutex() = default;
    inline void lock() {
        static const uint32_t spinlock = [](){
            const char* v = getenv("SPINLOCK");
            if ((v == nullptr) || (strcmp(v, "0") == 0)) {
                return false;
            }
            if (strcmp(v, "1") == 0) {
                return true;
            }
            throw std::runtime_error("Could not parse SPINLOCK variable");
        }();
        if (spinlock) {
            while (!try_lock());
        } else {
            // Spinlock for 25ms on a 4GHz processor
            for (uint32_t i = 0; i < 100'000'000; ++i) {
                if (try_lock()) {
                    return;
                }
            }
            while (!try_lock()) {
                Mlib::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    inline bool try_lock() {
        return must_lock();
    }
    inline bool must_lock() {
        return !flag_.test_and_set(std::memory_order_acquire);
    }
    inline void unlock() {
        flag_.clear(std::memory_order_release);
    }
private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

}
