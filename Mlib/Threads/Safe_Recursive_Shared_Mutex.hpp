#pragma once
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstdint>
#include <mutex>
#include <unordered_map>

namespace Mlib {

class SafeRecursiveSharedMutex {
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
    void unlock() {
        mutex_.unlock();
    }
    void lock_shared() {
        if (!mutex_.is_owner()) {
            std::scoped_lock lock{shared_owners_mutex_};
            if (++shared_owners_[std::this_thread::get_id()] > 1) {
                return;
            }
        }
        mutex_.lock_shared();
    }
    void unlock_shared() {
        if (!mutex_.is_owner()) {
            std::scoped_lock lock{shared_owners_mutex_};
            auto it = shared_owners_.find(std::this_thread::get_id());
            if (it == shared_owners_.end()) {
                THROW_OR_ABORT("Could not delete shared mutex info");
            }
            if (it->second == 1) {
                shared_owners_.erase(std::this_thread::get_id());
            } else {
                --it->second;
                return;
            }
        }
        mutex_.unlock_shared();
    }
private:
    RecursiveSharedMutex mutex_;
    std::mutex shared_owners_mutex_;
    std::unordered_map<std::thread::id, uint32_t> shared_owners_;
};

}
