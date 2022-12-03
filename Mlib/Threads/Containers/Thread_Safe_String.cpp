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
#ifdef __clang__
    if (str_ < other.str_) {
        return std::strong_ordering::less;
    }
    if (str_ == other.str_) {
        return std::strong_ordering::equal;
    }
    if (str_ > other.str_) {
        return std::strong_ordering::greater;
    }
    throw std::runtime_error("Invalid string comparison");
#else
    return str_ <=> other.str_;
#endif
}

ThreadSafeString::operator std::string() const {
    std::lock_guard lock{mutex_};
    return str_;
}
