#include "Checked_Mutex.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

using namespace Mlib;

CheckedMutex::CheckedMutex() = default;

CheckedMutex::~CheckedMutex() = default;

void CheckedMutex::lock() {
    if (locked_by_caller()) {
        THROW_OR_ABORT("Mutex already locked by caller");
    }
    mutex_.lock();
    holder_ = std::this_thread::get_id(); 
}

void CheckedMutex::unlock() {
    holder_ = std::thread::id();
    mutex_.unlock();
}

void CheckedMutex::lock_shared() {
    mutex_.lock_shared();
}

void CheckedMutex::unlock_shared() {
    mutex_.unlock_shared();
}

bool CheckedMutex::locked_by_caller() const {
    return holder_ == std::this_thread::get_id();
}

void CheckedMutex::assert_locked_by_caller() const {
    if (!locked_by_caller()) {
        THROW_OR_ABORT("Mutex not locked by caller");
    }
}
