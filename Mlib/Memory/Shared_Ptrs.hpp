#pragma once
#include <Mlib/Object.hpp>
#include <atomic>
#include <list>
#include <memory>
#include <mutex>

namespace Mlib {

class SharedPtrs {
public:
    SharedPtrs();
    ~SharedPtrs();
    void add(std::shared_ptr<Object> ptr);
    void clear();
private:
    std::list<std::shared_ptr<Object>> ptrs_;
    mutable std::mutex mutex_;
    std::atomic_bool clearing_;
};

};
