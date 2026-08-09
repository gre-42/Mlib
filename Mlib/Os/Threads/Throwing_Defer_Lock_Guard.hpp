#pragma once
#include <stdexcept>

namespace Mlib {

template <class TMutex>
class ThrowingDeferLockGuard {
public:
    ThrowingDeferLockGuard()
        : mutex_{nullptr}
    {}
    void lock(TMutex& mutex) {
        if (mutex_ != nullptr) {
            throw std::runtime_error("Mutex already locked");
        }
        if (!mutex.must_lock()) {
            throw std::runtime_error("Could not lock mutex");
        }
        mutex_ = &mutex;
    }
    ~ThrowingDeferLockGuard() {
        if (mutex_ != nullptr) {
            mutex_->unlock();
        }
    }
private:
    TMutex* mutex_;
};

}
