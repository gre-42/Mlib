#pragma once
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <vector>

namespace Mlib {

template <class TBaseMap>
class VerboseGenericMap {
public:
    using value_type = TBaseMap::value_type;
    using key_type = TBaseMap::key_type;
    using mapped_type = TBaseMap::mapped_type;
    using node_type = TBaseMap::node_type;
    using iterator = TBaseMap::iterator;
    using const_iterator = TBaseMap::const_iterator;

    VerboseGenericMap(
        std::string value_name,
        std::function<std::string(const key_type& e)> key_to_string)
        : value_name_{ std::move(value_name) }
        , key_to_string_{ std::move(key_to_string) } {
    }

    VerboseGenericMap(
        std::string value_name,
        std::function<std::string(const key_type& e)> key_to_string,
        std::initializer_list<value_type> l)
        : elements_{ std::move(l) }
        , value_name_{ std::move(value_name) }
        , key_to_string_{ std::move(key_to_string) } {
    }

    bool empty() const {
        return elements_.empty();
    }

    size_t size() const {
        return elements_.size();
    }

    iterator begin() {
        return elements_.begin();
    }

    iterator end() {
        return elements_.end();
    }

    const_iterator begin() const {
        return elements_.begin();
    }

    const_iterator end() const {
        return elements_.end();
    }

    bool contains(const key_type& key) const {
        return elements_.contains(key);
    }

    bool contains(const key_type& key, const mapped_type& value, bool deflt = false) const {
        auto it = elements_.find(key);
        if (it == elements_.end()) {
            return deflt;
        } else {
            return it->second == value;
        }
    }

    node_type extract(const key_type& key) {
        auto res = elements_.extract(key);
        if (res.empty()) {
            THROW_OR_ABORT(value_name_ + " with name \"" + key_to_string_(key) + "\" does not exist");
        }
        return res;
    }

    node_type try_extract(const key_type& key) {
        return elements_.extract(key);
    }

    const mapped_type& operator[](const key_type& key) const {
        return elements_[key];
    }

    mapped_type& operator[](const key_type& key) {
        return elements_[key];
    }

    const mapped_type& get(const key_type& key) const {
        return const_cast<VerboseGenericMap*>(this)->get(key);
    }

    mapped_type& get(const key_type& key) {
        auto it = elements_.find(key);
        if (it == elements_.end()) {
            THROW_OR_ABORT(value_name_ + " with key \"" + key_to_string_(key) +
                "\" does not exist");
        }
        return it->second;
    }

    mapped_type get(const key_type& key, const mapped_type& deflt) const {
        auto it = elements_.find(key);
        if (it == elements_.end()) {
            return deflt;
        }
        return it->second;
    }

    mapped_type* try_get(const key_type& key) {
        auto it = elements_.find(key);
        if (it == elements_.end()) {
            return nullptr;
        }
        return &it->second;
    }

    const mapped_type* try_get(const key_type& key) const {
        return const_cast<VerboseGenericMap*>(this)->try_get(key);
    }

    template< class... Args >
    mapped_type& add(const key_type& key, Args&&... args) {
        auto res = elements_.try_emplace(key, std::forward<Args>(args)...);
        if (!res.second) {
            THROW_OR_ABORT("Could not insert into map");
        }
        return res.first->second;
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

    void remove(const key_type& key) {
        if (erase(key) != 1) {
            verbose_abort(value_name_ + " with key \"" + key_to_string_(key) + "\" could not be removed");
        }
    }

    std::vector<key_type> keys() const {
        std::vector<key_type> result;
        result.reserve(this->size());
        for (const auto& [k, _] : *this) {
            result.push_back(k);
        }
        return result;
    }

private:
    TBaseMap elements_;
    std::string value_name_;
    std::function<std::string(const key_type& e)> key_to_string_;
};

}
