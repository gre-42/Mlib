#include "Safe_Recursive_Shared_Mutex.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

SafeRecursiveSharedMutex::SafeRecursiveSharedMutex(size_t max_map_size)
    : max_map_size_{ max_map_size }
{}

SafeRecursiveSharedMutex::~SafeRecursiveSharedMutex() = default;

void SafeRecursiveSharedMutex::lock() {
    throw_if_mutex_upgrade();
    mutex_.lock();
}

void SafeRecursiveSharedMutex::unlock() {
    mutex_.unlock();
}

bool SafeRecursiveSharedMutex::try_lock() {
    throw_if_mutex_upgrade();
    return mutex_.try_lock();
}

void SafeRecursiveSharedMutex::lock_shared() {
    if (!mutex_.is_owner()) {
        std::scoped_lock lock{ shared_owners_mutex_ };
        if (++shared_owners_[std::this_thread::get_id()] > 1) {
            return;
        }
    }
    mutex_.lock_shared();
}

void SafeRecursiveSharedMutex::unlock_shared() {
    if (!mutex_.is_owner()) {
        std::scoped_lock lock{ shared_owners_mutex_ };
        auto it = shared_owners_.find(std::this_thread::get_id());
        if (it == shared_owners_.end()) {
            verbose_abort("Could not delete shared mutex info");
        }
        if (it->second == 1) {
            if (shared_owners_.size() > max_map_size_) {
                shared_owners_.erase(std::this_thread::get_id());
            } else {
                it->second = 0;
            }
        } else {
            if (it->second == 0) {
                verbose_abort("Too many mutex unlocks");
            }
            --it->second;
            return;
        }
    }
    mutex_.unlock_shared();
}

void SafeRecursiveSharedMutex::assert_locked() const {
    if (!mutex_.is_owner()) {
        THROW_OR_ABORT("Mutex not locked");
    }
}

void SafeRecursiveSharedMutex::assert_locked_or_shared() const {
    std::scoped_lock lock{ shared_owners_mutex_ };
    if (!mutex_.is_owner() && !shared_owners_.contains(std::this_thread::get_id())) {
        THROW_OR_ABORT("Mutex not locked or shared");
    }
}

void SafeRecursiveSharedMutex::throw_if_mutex_upgrade() const {
    std::scoped_lock lock{ shared_owners_mutex_ };
    auto it = shared_owners_.find(std::this_thread::get_id());
    if ((it != shared_owners_.end()) && (it->second != 0)) {
        verbose_abort("Mutex upgrade not supported");
    }
}
