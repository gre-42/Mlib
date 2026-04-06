#pragma once
#include <functional>
#include <list>
#include <stdexcept>

namespace Mlib {

class DestructionGuards {
    DestructionGuards(const DestructionGuards&) = delete;
    DestructionGuards &operator=(const DestructionGuards&) = delete;
public:
    DestructionGuards();
    ~DestructionGuards();
    void add(std::function<void()> &&f);

private:
    std::list<std::function<void()>> f_;
};

}
