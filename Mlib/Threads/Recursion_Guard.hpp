#pragma once

namespace Mlib {

class RecursionCounter {
public:
    RecursionCounter();
    void operator ++();
    void operator --();
private:
    unsigned int recursion_counter_;
};

class RecursionGuard {
public:
    explicit RecursionGuard(RecursionCounter& recursion_counter);
    ~RecursionGuard();
private:
    RecursionCounter& recursion_counter_;
};

}
