#include <atomic>
#include <shared_mutex>
#include <thread>

namespace Mlib {

// From: https://stackoverflow.com/a/30109512/2292832
class CheckedMutex : public std::shared_mutex {
public:
    void lock();
    void unlock();
    bool locked_by_caller() const;
    void assert_locked_by_caller() const;

private:
    std::atomic<std::thread::id> m_holder;
};

}
