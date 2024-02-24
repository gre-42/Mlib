#pragma once
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <functional>
#include <map>
#include <mutex>
#include <string>

namespace Mlib {

template <class TKey, class TValue>
class ThreadsafeMap {
public:
    ThreadsafeMap(std::string value_name,
        std::function<std::string(const TKey& e)> element_to_string)
        : value_name_{ std::move(value_name) }
        , key_to_string_{ std::move(element_to_string) } {
    }
    ~ThreadsafeMap() = default;

    template <class... Args>
    TValue& emplace(TKey key, Args &&...args) {
        std::scoped_lock lock{ mutex_ };
        auto res = elements_.try_emplace(std::move(key), std::forward<Args>(args)...);
        if (!res.second) {
            THROW_OR_ABORT(value_name_ + " with key \"" + key_to_string_(key) +
                "\" already exists");
        }
        return res.first->second;
    }

    void set(const TKey& key, const TValue& value) {
        std::scoped_lock lock{ mutex_ };
        elements_[key] = value;
    }

    void clear() {
        std::scoped_lock lock{ mutex_ };
        elements_.clear();
    }

    size_t erase(const TKey& key) {
        std::scoped_lock lock{ mutex_ };
        return elements_.erase(key);
    }

    template <class TPredicate>
    void erase_if(const TPredicate& predicate) {
        std::scoped_lock lock{ mutex_ };
        std::erase_if(elements_, predicate);
    }

    bool contains(const TKey& key) const {
        std::shared_lock lock{ mutex_ };
        return elements_.contains(key);
    }

    const TValue* try_get(const TKey& key) const {
        std::shared_lock lock{ mutex_ };
        auto it = elements_.find(key);
        if (it == elements_.end()) {
            return nullptr;
        }
        return &it->second;
    }

    typename std::map<TKey, TValue>::node_type extract(const TKey& key) {
        std::shared_lock lock{ mutex_ };
        auto res = elements_.extract(key);
        if (res.empty()) {
            THROW_OR_ABORT(value_name_ + " with name \"" + key + "\" does not exist");
        }
        return res;
    }

    typename std::map<TKey, TValue>::node_type try_extract(const TKey& key) {
        std::shared_lock lock{ mutex_ };
        return elements_.extract(key);
    }

    TValue& get(const TKey& key) {
        std::shared_lock lock{ mutex_ };
        auto it = elements_.find(key);
        if (it == elements_.end()) {
            THROW_OR_ABORT(value_name_ + " with key \"" + key_to_string_(key) +
                "\" does not exist");
        }
        return it->second;
    }

    const TValue& get(const TKey& key) const {
        return const_cast<ThreadsafeMap*>(this)->get(key);
    }

    auto begin() { return elements_.begin(); }
    auto end() { return elements_.end(); }
    auto begin() const { return elements_.begin(); }
    auto end() const { return elements_.end(); }
private:
    mutable SafeRecursiveSharedMutex mutex_;
    std::map<TKey, TValue> elements_;
    std::string value_name_;
    std::function<std::string(const TKey& e)> key_to_string_;
};

}
