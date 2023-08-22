#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <unordered_set>

namespace Mlib {

class SafeSharedMutex {
public:
    void lock() {
        {
            std::scoped_lock lock{shared_owners_mutex_};
            if (shared_owners_.contains(std::this_thread::get_id())) {
                THROW_OR_ABORT("Mutex upgrade not supported");
            }
        }
        mutex_.lock();
    }
    bool try_lock() {
        {
            std::scoped_lock lock{shared_owners_mutex_};
            if (shared_owners_.contains(std::this_thread::get_id())) {
                THROW_OR_ABORT("Mutex upgrade not supported");
            }
        }
        return mutex_.try_lock();
    }
    void unlock() {
        mutex_.unlock();
    }
    void lock_shared() {
        {
            std::scoped_lock lock{shared_owners_mutex_};
            if (!shared_owners_.insert(std::this_thread::get_id()).second) {
                THROW_OR_ABORT("Mutex is already locked");
            }
        }
        mutex_.lock_shared();
    }
    void unlock_shared() {
        {
            std::scoped_lock lock{shared_owners_mutex_};
            if (shared_owners_.erase(std::this_thread::get_id()) != 1) {
                THROW_OR_ABORT("Could not delete shared mutex info");
            }
        }
        mutex_.unlock_shared();
    }
private:
    std::shared_mutex mutex_;
    std::mutex shared_owners_mutex_;
    std::unordered_set<std::thread::id> shared_owners_;
};

}
