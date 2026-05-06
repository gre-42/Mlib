#include "Destruction_Guards.hpp"

using namespace Mlib;

DestructionGuards::DestructionGuards() = default;

DestructionGuards::~DestructionGuards() {
    for (const auto &f : f_) {
        f();
    }
}

void DestructionGuards::add(std::function<void()> &&f) {
    f_.emplace_front(std::move(f));
}
