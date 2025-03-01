#pragma once
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <cstdint>
#include <mutex>
#include <unordered_map>

namespace Mlib {

class SafeRecursiveSharedMutex {
public:
    explicit SafeRecursiveSharedMutex(size_t max_map_size = 1'000);
    ~SafeRecursiveSharedMutex();
    void lock();
    void unlock();
    bool try_lock();
    void lock_shared();
    void unlock_shared();
    void assert_locked() const;
    void assert_locked_or_shared() const;
private:
    void throw_if_mutex_upgrade() const;
    RecursiveSharedMutex mutex_;
    mutable FastMutex shared_owners_mutex_;
    std::unordered_map<std::thread::id, uint32_t> shared_owners_;
    size_t max_map_size_;
};

}
