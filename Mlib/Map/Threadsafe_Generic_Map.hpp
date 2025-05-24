#pragma once
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <functional>
#include <mutex>
#include <string>

namespace Mlib {

template <class TBaseMap>
class ThreadsafeGenericMap {
public:
    using value_type = TBaseMap::value_type;
    using key_type = TBaseMap::key_type;
    using mapped_type = TBaseMap::mapped_type;
    using node_type = TBaseMap::node_type;

    // Empty constructor
    ThreadsafeGenericMap(
        std::string value_name,
        std::function<std::string(const key_type& e)> element_to_string)
        : value_name_{ std::move(value_name) }
        , key_to_string_{ std::move(element_to_string) } {
    }

    // Initializer list
    ThreadsafeGenericMap(
        std::string value_name,
        std::function<std::string(const key_type& e)> element_to_string,
        std::initializer_list<value_type>&& l)
        : elements_{ l }
        , value_name_{ std::move(value_name) }
        , key_to_string_{ std::move(element_to_string) } {
    }

    // Move constructor and move assignment operator
    ThreadsafeGenericMap(ThreadsafeGenericMap&& other)
        : value_name_{ std::move(other.value_name) }
        , key_to_string_{ std::move(other.key_to_string) }
    {
        // No need to lock this->mutex_ here
        // (in contrast to the move assignment operator below).
        std::scoped_lock lock{ other.mutex_ };
        elements_ = std::move(other.elements_);
    }

    ThreadsafeGenericMap& operator = (ThreadsafeGenericMap&& other)
    {
        std::scoped_lock lock{ mutex_, other.mutex_ };
        elements_ = std::move(other.elements_);
        value_name_ = std::move(other.value_name_);
        key_to_string_ = std::move(other.key_to_string_);
        return *this;
    }
    
    // Destructor
    ~ThreadsafeGenericMap() = default;

    template <class... Args>
    mapped_type& add(key_type key, Args &&...args) {
        std::scoped_lock lock{ mutex_ };
        auto res = elements_.try_emplace(std::move(key), std::forward<Args>(args)...);
        if (!res.second) {
            THROW_OR_ABORT(value_name_ + " with key \"" + key_to_string_(key) +
                "\" already exists");
        }
        return res.first->second;
    }

    mapped_type& add_node(node_type&& node) {
        std::shared_lock lock{ mutex_ };
        auto it = elements_.insert(std::move(node));
        if (!it.inserted) {
            THROW_OR_ABORT(value_name_ + " with name \"" + key_to_string_(node.key()) + "\" already exists");
        }
        return it.position->second;
    }

    decltype(auto) insert_or_assign(const key_type& key, const mapped_type& value) {
        std::scoped_lock lock{ mutex_ };
        return elements_.insert_or_assign(key, value);
    }

    void clear() {
        std::scoped_lock lock{ mutex_ };
        elements_.clear();
    }

    size_t erase(const key_type& key) {
        std::scoped_lock lock{ mutex_ };
        return elements_.erase(key);
    }

    template <class TPredicate>
    void erase_if(const TPredicate& predicate) {
        std::scoped_lock lock{ mutex_ };
        std::erase_if(elements_, predicate);
    }

    bool contains(const key_type& key) const {
        std::shared_lock lock{ mutex_ };
        return elements_.contains(key);
    }

    const mapped_type* try_get(const key_type& key) const {
        std::shared_lock lock{ mutex_ };
        auto it = elements_.find(key);
        if (it == elements_.end()) {
            return nullptr;
        }
        return &it->second;
    }

    node_type extract(const key_type& key) {
        std::scoped_lock lock{ mutex_ };
        auto res = elements_.extract(key);
        if (res.empty()) {
            THROW_OR_ABORT(value_name_ + " with name \"" + key_to_string_(key) + "\" does not exist");
        }
        return res;
    }

    node_type try_extract(const key_type& key) {
        std::scoped_lock lock{ mutex_ };
        return elements_.extract(key);
    }

    mapped_type& get(const key_type& key) {
        std::shared_lock lock{ mutex_ };
        auto it = elements_.find(key);
        if (it == elements_.end()) {
            THROW_OR_ABORT(value_name_ + " with key \"" + key_to_string_(key) +
                "\" does not exist");
        }
        return it->second;
    }

    const mapped_type& get(const key_type& key) const {
        return const_cast<ThreadsafeGenericMap*>(this)->get(key);
    }

    bool empty() const { return elements_.empty(); }
    size_t size() const { return elements_.size(); }
    
    decltype(auto) begin() { return elements_.begin(); }
    decltype(auto) end() { return elements_.end(); }
    decltype(auto) begin() const { return elements_.begin(); }
    decltype(auto) end() const { return elements_.end(); }
private:
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    TBaseMap elements_;
    std::string value_name_;
    std::function<std::string(const key_type& e)> key_to_string_;
};

}
