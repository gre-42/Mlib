#pragma once
#include <mutex>

namespace Mlib {

class MustLockMutex: public std::timed_mutex {
public:
    inline bool must_lock() {
        return try_lock_for(std::chrono::milliseconds(500));
    }
};

}
