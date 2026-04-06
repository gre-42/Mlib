#pragma once
#include <stdexcept>

namespace Mlib {

template <class TMutex>
class ThrowingLockGuard {
public:
    explicit ThrowingLockGuard(TMutex& mutex)
        : mutex_{mutex}
    {
        if (!mutex_.must_lock()) {
            throw std::runtime_error("Could not lock mutex");
        }
    }
    ~ThrowingLockGuard() {
        mutex_.unlock();
    }
private:
    TMutex& mutex_;
};

}
