#pragma once
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <compare>
#include <nlohmann/json.hpp>

namespace Mlib {

class ThreadSafeJson {
public:
    ThreadSafeJson();
    ThreadSafeJson(const ThreadSafeJson& other);
    ThreadSafeJson(ThreadSafeJson&& other) noexcept;
    ThreadSafeJson& operator = (const ThreadSafeJson& other);
    ThreadSafeJson& operator = (nlohmann::json other);
    std::partial_ordering operator <=> (const ThreadSafeJson& other) const;
    bool operator == (const ThreadSafeJson& other) const;
    bool operator != (const ThreadSafeJson& other) const;
    nlohmann::json json() const;
    template <class T>
    T get() const {
        std::scoped_lock lock{ mutex_ };
        return j_.get<T>();
    }
private:
    nlohmann::json j_;
    mutable FastMutex mutex_;
};

void from_json(const nlohmann::json& j, ThreadSafeJson& t);
void to_json(nlohmann::json& j, const ThreadSafeJson& t);

}
