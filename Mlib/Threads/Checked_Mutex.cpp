#include "Checked_Mutex.hpp"
#include <stdexcept>

using namespace Mlib;

void CheckedMutex::lock() {
    if (locked_by_caller()) {
        throw std::runtime_error("Mutex already locked by caller");
    }
    std::mutex::lock();
    m_holder = std::this_thread::get_id(); 
}

void CheckedMutex::unlock() {
    m_holder = std::thread::id();
    std::mutex::unlock();
}

bool CheckedMutex::locked_by_caller() const {
    return m_holder == std::this_thread::get_id();
}

void CheckedMutex::assert_locked_by_caller() const {
    if (!locked_by_caller()) {
        throw std::runtime_error("Mutex not locked by caller");
    }
}
