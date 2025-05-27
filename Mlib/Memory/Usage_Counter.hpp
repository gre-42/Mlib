#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Source_Location.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <atomic>
#include <cstddef>

namespace Mlib {

class UsageCounter;

class CounterUser {
    friend UsageCounter;
    CounterUser(const CounterUser&) = delete;
    CounterUser& operator = (const CounterUser&) = delete;
public:
    explicit CounterUser(const DanglingBaseClassRef<UsageCounter>& counter);
    ~CounterUser();
    void set(bool value);
private:
    DanglingBaseClassRef<UsageCounter> usage_counter_;
    bool value_;
    FastMutex mutex_;
};

class UsageCounter: public virtual DanglingBaseClass {
    UsageCounter(const UsageCounter&) = delete;
    UsageCounter& operator = (const UsageCounter&) = delete;
public:
    UsageCounter();
    ~UsageCounter();
    void increase();
    void decrease();
    size_t count() const;
private:
    std::atomic_size_t count_;
};

}
