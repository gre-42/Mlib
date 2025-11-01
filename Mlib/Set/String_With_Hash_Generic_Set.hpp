#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <string>
#include <vector>

namespace Mlib {

template <class TBaseMap>
class StringWithHashGenericSet {
public:
    using value_type = TBaseMap::value_type;
    using key_type = TBaseMap::key_type;
    using node_type = TBaseMap::node_type;
    using iterator = TBaseMap::iterator;
    using const_iterator = TBaseMap::const_iterator;

    explicit StringWithHashGenericSet(std::string value_name)
        : value_name_{ std::move(value_name) }
    {}
    ~StringWithHashGenericSet() = default;

    iterator add(key_type key) {
        auto res = elements_.insert(std::move(key));
        if (!res.second) {
            THROW_OR_ABORT(value_name_ + " with name \"" + *key + "\" already exists");
        }
        return res.first;
    }

    template <class... Args>
    auto try_add(key_type key) {
        return elements_.insert(std::move(key));
    }

    decltype(auto) insert(node_type&& node) {
        return elements_.insert(std::move(node));
    }

    void clear() {
        elements_.clear();
    }

    decltype(auto) find(const key_type& key) {
        return elements_.find(key);
    }

    decltype(auto) find(const key_type& key) const {
        return elements_.find(key);
    }

    void erase(const const_iterator& it) {
        elements_.erase(it);
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

    node_type extract(const const_iterator& it) {
        return elements_.extract(it);
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

    // template <class Archive>
    // void serialize(Archive& archive) {
    //     archive(value_name_);
    //     archive(elements_);
    // }
    // 
    // template<typename Archive>
    // static void load_and_construct(
    //     Archive& archive,
    //     cereal::construct<StringWithHashGenericSet<TBaseMap>> &construct)
    // {
    //     std::string value_name;
    //     archive(value_name);
    //     auto& obj = construct(value_name);
    //     archive(obj.elements_);
    // }
    TBaseMap& elements() {
        return elements_;
    }
    const TBaseMap& elements() const {
        return elements_;
    }
private:
    TBaseMap elements_;
    std::string value_name_;
};

}
