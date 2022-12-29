#include "Checked_Mutex.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

using namespace Mlib;

void CheckedMutex::lock() {
    if (locked_by_caller()) {
        THROW_OR_ABORT("Mutex already locked by caller");
    }
    std::shared_mutex::lock();
    m_holder = std::this_thread::get_id(); 
}

void CheckedMutex::unlock() {
    m_holder = std::thread::id();
    std::shared_mutex::unlock();
}

bool CheckedMutex::locked_by_caller() const {
    return m_holder == std::this_thread::get_id();
}

void CheckedMutex::assert_locked_by_caller() const {
    if (!locked_by_caller()) {
        THROW_OR_ABORT("Mutex not locked by caller");
    }
}
