#pragma once
#include <compare>
#include <mutex>
#include <string>

namespace Mlib {

class ThreadSafeString {
public:
    ThreadSafeString();
    ThreadSafeString& operator = (const std::string& other);
    std::strong_ordering operator <=> (const ThreadSafeString& other) const;
    explicit operator std::string() const;
private:
    std::string str_;
    mutable std::mutex mutex_;
};

}
