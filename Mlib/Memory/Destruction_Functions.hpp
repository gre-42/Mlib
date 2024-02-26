#pragma once
#include <atomic>
#include <functional>
#include <list>
#include <mutex>

namespace Mlib {

class DestructionFunctions {
    DestructionFunctions(const DestructionFunctions&) = delete;
    DestructionFunctions& operator = (const DestructionFunctions&) = delete;
public:
    DestructionFunctions();
    ~DestructionFunctions();
    void add(std::function<void()> f);
    void clear();
private:
    std::list<std::function<void()>> funcs_;
    std::mutex mutex_;
    std::atomic_bool clearing_;
};

}
