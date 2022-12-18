#pragma once
#include <Mlib/Deallocation_Token.hpp>
#include <functional>
#include <list>
#include <mutex>

namespace Mlib {

class DeallocationToken;

class Deallocators {
    friend DeallocationToken;
public:
    DeallocationToken insert(const std::function<void()>& deallocate);
    void erase(const std::list<std::function<void()>>::iterator& token);
    void deallocate();
private:
    std::list<std::function<void()>> deallocators_;
    mutable std::mutex mutex_;
};

}
