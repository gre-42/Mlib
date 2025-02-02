#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <functional>
#include <list>
#include <mutex>

namespace Mlib {

class DeallocationToken;

class Deallocators {
    Deallocators(const Deallocators&) = delete;
    Deallocators& operator = (const Deallocators&) = delete;
public:
    Deallocators();
    ~Deallocators();
    DeallocationToken insert(const std::function<void()>& deallocate);
    void erase(const std::list<std::function<void()>>::iterator& token);
    void deallocate();
private:
    std::list<std::function<void()>> deallocators_;
    mutable std::recursive_mutex mutex_;
};

}
