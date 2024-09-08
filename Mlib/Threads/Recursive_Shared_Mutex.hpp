#pragma once
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <shared_mutex>
#include <thread>

namespace Mlib {

template <class TMutex>
class GenericRecursiveSharedMutex {
public:
    GenericRecursiveSharedMutex()
        : owner_{ std::thread::id() }
        , count_{ 0 }
    {}
    ~GenericRecursiveSharedMutex() = default;
    void lock() {
        if (!is_owner()) {
            mutex_.lock();
            owner_ = std::this_thread::get_id();
        }
        ++count_;
    }
    bool try_lock() {
        if (!is_owner()) {
            if (!mutex_.try_lock()) {
                return false;
            }
            owner_ = std::this_thread::get_id();
        }
        ++count_;
        return true;
    }
    void unlock() {
        --count_;
        if (count_ == 0) {
            owner_ = std::thread::id();
            mutex_.unlock();
        }
    }
    void lock_shared() {
        if (!is_owner()) {
            mutex_.lock_shared();
        }
    }
    void unlock_shared() {
        if (!is_owner()) {
            mutex_.unlock_shared();
        }
    }
    bool is_owner() const {
        return (owner_ == std::this_thread::get_id());
    }
private:
    TMutex mutex_;
    std::atomic<std::thread::id> owner_;
    std::atomic_uint32_t count_;
};

using RecursiveSharedMutex = GenericRecursiveSharedMutex<std::shared_mutex>;
using SafeAtomicRecursiveSharedMutex = GenericRecursiveSharedMutex<SafeAtomicSharedMutex>;

}
