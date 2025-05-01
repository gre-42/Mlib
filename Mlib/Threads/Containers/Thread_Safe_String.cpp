#include "Thread_Safe_String.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

ThreadSafeString::ThreadSafeString() = default;

ThreadSafeString::ThreadSafeString(const ThreadSafeString& other) {
    std::scoped_lock lock{ mutex_, other.mutex_ };
    str_ = other.str_;
}

ThreadSafeString::ThreadSafeString(ThreadSafeString&& other) noexcept {
    std::scoped_lock lock{ mutex_, other.mutex_ };
    str_ = std::move(other.str_);
}

ThreadSafeString& ThreadSafeString::operator = (const ThreadSafeString& other) {
    *this = (std::string)other;
    return *this;
}

ThreadSafeString& ThreadSafeString::operator = (std::string other) {
    std::scoped_lock lock{ mutex_ };
    str_ = std::move(other);
    return *this;
}

bool ThreadSafeString::operator == (const ThreadSafeString& other) const {
    std::scoped_lock lock{ mutex_, other.mutex_ };
    return str_ == other.str_;
}

bool ThreadSafeString::operator != (const ThreadSafeString& other) const {
    std::scoped_lock lock{ mutex_, other.mutex_ };
    return str_ != other.str_;
}

std::strong_ordering ThreadSafeString::operator <=> (const ThreadSafeString& other) const {
    std::scoped_lock lock{ mutex_, other.mutex_ };
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
    std::scoped_lock lock{ mutex_ };
    return str_;
}
