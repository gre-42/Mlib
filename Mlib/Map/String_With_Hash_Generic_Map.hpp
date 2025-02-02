#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <string>

namespace Mlib {

template <class TBaseMap>
class StringWithHashGenericMap {
public:
    using mapped_type = TBaseMap::mapped_type;
    using key_type = TBaseMap::key_type;
    using node_type = TBaseMap::node_type;

    explicit StringWithHashGenericMap(std::string value_name)
        : value_name_{ std::move(value_name) }
    {}
    ~StringWithHashGenericMap() = default;

    template <class... Args>
    mapped_type& add(key_type key, Args &&...args) {
        auto res = elements_.try_emplace(std::move(key), std::forward<Args>(args)...);
        if (!res.second) {
            THROW_OR_ABORT(value_name_ + " with name \"" + *key + "\" already exists");
        }
        return res.first->second;
    }

    template <class... Args>
    auto try_emplace(key_type key, Args &&...args) {
        return elements_.try_emplace(std::move(key), std::forward<Args>(args)...);
    }

    void insert_or_assign(const key_type& key, const mapped_type& value) {
        elements_.insert_or_assign(key, value);
    }

    void clear() {
        elements_.clear();
    }

    size_t erase(const key_type& key) {
        return elements_.erase(key);
    }

    template <class TPredicate>
    void erase_if(const TPredicate& predicate) {
        std::erase_if(elements_, predicate);
    }

    bool contains(const key_type& key) const {
        return elements_.contains(key);
    }

    const mapped_type* try_get(const key_type& key) const {
        auto it = elements_.find(key);
        if (it == elements_.end()) {
            return nullptr;
        }
        return &it->second;
    }

    node_type extract(const key_type& key) {
        auto res = elements_.extract(key);
        if (res.empty()) {
            THROW_OR_ABORT(value_name_ + " with name \"" + *key + "\" does not exist");
        }
        return res;
    }

    node_type try_extract(const key_type& key) {
        return elements_.extract(key);
    }

    mapped_type& get(const key_type& key) {
        auto it = elements_.find(key);
        if (it == elements_.end()) {
            THROW_OR_ABORT(value_name_ + " with name \"" + *key + "\" does not exist");
        }
        return it->second;
    }

    const mapped_type& get(const key_type& name) const {
        return const_cast<StringWithHashGenericMap*>(this)->get(name);
    }

    bool empty() const {
        return elements_.empty();
    }

    size_t size() const {
        return elements_.size();
    }

    decltype(auto) begin() { return elements_.begin(); }
    decltype(auto) end() { return elements_.end(); }
    decltype(auto) begin() const { return elements_.begin(); }
    decltype(auto) end() const { return elements_.end(); }
private:
    TBaseMap elements_;
    std::string value_name_;
};

}
