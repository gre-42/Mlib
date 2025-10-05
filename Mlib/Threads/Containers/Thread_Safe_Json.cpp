#include "Thread_Safe_Json.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

ThreadSafeJson::ThreadSafeJson() = default;

ThreadSafeJson::ThreadSafeJson(const ThreadSafeJson& other) {
    std::scoped_lock lock{ mutex_, other.mutex_ };
    j_ = other.j_;
}

ThreadSafeJson::ThreadSafeJson(ThreadSafeJson&& other) noexcept {
    std::scoped_lock lock{ mutex_, other.mutex_ };
    j_ = std::move(other.j_);
}

ThreadSafeJson& ThreadSafeJson::operator = (const ThreadSafeJson& other) {
    *this = (nlohmann::json)other;
    return *this;
}

ThreadSafeJson& ThreadSafeJson::operator = (nlohmann::json other) {
    std::scoped_lock lock{ mutex_ };
    j_ = std::move(other);
    return *this;
}

bool ThreadSafeJson::operator == (const ThreadSafeJson& other) const {
    std::scoped_lock lock{ mutex_, other.mutex_ };
    return j_ == other.j_;
}

bool ThreadSafeJson::operator != (const ThreadSafeJson& other) const {
    std::scoped_lock lock{ mutex_, other.mutex_ };
    return j_ != other.j_;
}

std::partial_ordering ThreadSafeJson::operator <=> (const ThreadSafeJson& other) const {
    std::scoped_lock lock{ mutex_, other.mutex_ };
#ifdef __clang__
    if (j_ < other.j_) {
        return std::partial_ordering::less;
    }
    if (j_ == other.j_) {
        return std::partial_ordering::equal;
    }
    if (j_ > other.j_) {
        return std::partial_ordering::greater;
    }
    THROW_OR_ABORT("Invalid string comparison");
#else
    return j_ <=> other.j_;
#endif
}

nlohmann::json ThreadSafeJson::json() const {
    std::scoped_lock lock{ mutex_ };
    return j_;
}

void Mlib::from_json(const nlohmann::json& j, ThreadSafeJson& t) {
    t = j;
}

void Mlib::to_json(nlohmann::json& j, const ThreadSafeJson& t) {
    j = t.json();
}
