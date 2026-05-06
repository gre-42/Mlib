#include "Shared_Ptrs.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Os/Os.hpp>
#include <mutex>

using namespace Mlib;

SharedPtrs::SharedPtrs()
    : clearing_{ false }
{}

SharedPtrs::~SharedPtrs() {
    clear();
}

void SharedPtrs::add(std::shared_ptr<Object> ptr) {
    if (clearing_) {
        verbose_abort("SharedPtrs::add called during clearing");
    }
    std::scoped_lock lock{ mutex_ };
    ptrs_.push_back(std::move(ptr));
}

void SharedPtrs::clear() {
    if (clearing_) {
        verbose_abort("SharedPtrs already clearing");
    }
    std::unique_lock lock{ mutex_ };
    clearing_ = true;
    clear_list_recursively_with_lock(ptrs_, lock, [](const auto&) {});
    clearing_ = false;
}
