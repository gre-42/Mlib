#include "Shared_Ptrs.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

SharedPtrs::SharedPtrs()
: clearing_{false}
{}

SharedPtrs::~SharedPtrs() {
    clear();
}

void SharedPtrs::add(std::shared_ptr<Object> ptr) {
    std::scoped_lock lock{mutex_};
    if (clearing_) {
        verbose_abort("SharedPtrs::add called during clearing");
    }
    ptrs_.push_back(std::move(ptr));
}

void SharedPtrs::clear() {
    std::scoped_lock lock{mutex_};
    if (clearing_) {
        verbose_abort("SharedPtrs already clearing");
    }
    clearing_ = true;
    clear_container_recursively(ptrs_);
    clearing_ = false;
}
