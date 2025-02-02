#pragma once
#include <Mlib/Object.hpp>
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <atomic>
#include <list>
#include <memory>

namespace Mlib {

class SharedPtrs {
public:
    SharedPtrs();
    ~SharedPtrs();
    void add(std::shared_ptr<Object> ptr);
    void clear();
private:
    std::list<std::shared_ptr<Object>> ptrs_;
    mutable AtomicMutex mutex_;
    std::atomic_bool clearing_;
};

};
