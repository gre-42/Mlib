#pragma once
#include <mutex>

namespace Mlib {

class DeleteRigidBodyMutex {
public:
    void lock() {
        mutex_.lock();
        ++nlocked_;
    }
    void unlock() {
        --nlocked_;
        mutex_.unlock();
    }
    bool is_locked() const {
        return nlocked_ > 0;
    }
private:
    std::recursive_mutex mutex_;
    unsigned int nlocked_;
};

}
