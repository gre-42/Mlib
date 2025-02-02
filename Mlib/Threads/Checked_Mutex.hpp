#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <atomic>
#include <thread>

namespace Mlib {

// From: https://stackoverflow.com/a/30109512/2292832
class CheckedMutex {
public:
    CheckedMutex();
    ~CheckedMutex();
    void lock();
    void unlock();
    void lock_shared();
    void unlock_shared();
    bool locked_by_caller() const;
    void assert_locked_by_caller() const;

private:
    std::atomic<std::thread::id> m_holder;
    SafeAtomicRecursiveSharedMutex mutex_;
};

}
