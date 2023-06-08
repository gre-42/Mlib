#include "Shared_Ptrs.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>

using namespace Mlib;

SharedPtrs::SharedPtrs() = default;

SharedPtrs::~SharedPtrs() {
    clear();
}

void SharedPtrs::add(std::shared_ptr<Object> ptr) {
    ptrs_.push_back(std::move(ptr));
}

void SharedPtrs::clear() {
    clear_container_recursively(ptrs_);
}
