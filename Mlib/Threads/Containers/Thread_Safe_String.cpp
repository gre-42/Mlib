#include "Thread_Safe_String.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

ThreadSafeString::ThreadSafeString() = default;

ThreadSafeString& ThreadSafeString::operator = (const std::string& other) {
    std::scoped_lock lock{mutex_};
    str_ = other;
    return *this;
}

std::strong_ordering ThreadSafeString::operator <=> (const ThreadSafeString& other) const {
    std::scoped_lock lock{mutex_};
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
    THROW_OR_ABORT("Invalid string comparison");
#else
    return str_ <=> other.str_;
#endif
}

ThreadSafeString::operator std::string() const {
    std::scoped_lock lock{mutex_};
    return str_;
}
