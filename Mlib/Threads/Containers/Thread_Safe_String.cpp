#include "Thread_Safe_String.hpp"

using namespace Mlib;

ThreadSafeString::ThreadSafeString()
{}

ThreadSafeString& ThreadSafeString::operator = (const std::string& other) {
    std::lock_guard lock{mutex_};
    str_ = other;
    return *this;
}

std::strong_ordering ThreadSafeString::operator <=> (const ThreadSafeString& other) const {
    std::lock_guard lock{mutex_};
    return str_ <=> other.str_;
}

ThreadSafeString::operator std::string() const {
    std::lock_guard lock{mutex_};
    return str_;
}
