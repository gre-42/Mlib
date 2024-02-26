#include "Destruction_Functions.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

DestructionFunctions::DestructionFunctions()
    : clearing_{ false }
{}

DestructionFunctions::~DestructionFunctions() {
    clear();
}

void DestructionFunctions::add(std::function<void()> f) {
    if (clearing_) {
        verbose_abort("DestructionFunctions::add called during clearing");
    }
    std::scoped_lock lock{ mutex_ };
    funcs_.emplace_back(std::move(f));
}

void DestructionFunctions::clear() {
    if (clearing_) {
        verbose_abort("DestructionFunctions already clearing");
    }
    std::unique_lock lock{ mutex_ };
    clearing_ = true;
    clear_list_recursively_with_lock(funcs_, lock, [](auto& f) { f(); });
    clearing_ = false;
}
