#pragma once
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <compare>
#include <map>
#include <stdexcept>
#include <vector>

namespace Mlib {

template <class TBaseMap>
class GenericMap: public TBaseMap {
public:
    using value_type = TBaseMap::value_type;
    using key_type = TBaseMap::key_type;
    using mapped_type = TBaseMap::mapped_type;
    using node_type = TBaseMap::node_type;

    GenericMap() {}
    GenericMap(std::initializer_list<value_type> init)
        : TBaseMap(init)
    {}

    bool contains(const key_type& key) const {
        const TBaseMap* m = this;
        return m->contains(key);
    }

    bool contains(const key_type& key, const mapped_type& value, bool deflt = false) const {
        auto it = this->find(key);
        if (it == this->end()) {
            return deflt;
        } else {
            return it->second == value;
        }
    }

    const mapped_type& get(const key_type& key) const {
        return const_cast<GenericMap*>(this)->get(key);
    }

    mapped_type& get(const key_type& key) {
        auto it = this->find(key);
        if (it == this->end()) {
            THROW_OR_ABORT("Could not find entry with key \"" + key + '"');
        }
        return it->second;
    }

    mapped_type get(const key_type& key, const mapped_type& deflt) const {
        auto it = this->find(key);
        if (it == this->end()) {
            return deflt;
        }
        return it->second;
    }

    const mapped_type* try_get(const key_type& key) const {
        auto it = this->find(key);
        if (it == this->end()) {
            return nullptr;
        }
        return &it->second;
    }

    template< class... Args >
    mapped_type& add(const key_type& key, Args&&... args) {
        auto res = this->try_emplace(key, std::forward<Args>(args)...);
        if (!res.second) {
            THROW_OR_ABORT("Could not insert into map");
        }
        return res.first->second;
    }

    void remove(const key_type& key) {
        if (this->erase(key) != 1) {
            verbose_abort("Could not remove element");
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

    decltype(auto) operator <=> (const GenericMap& other) const {
        const TBaseMap& a = *this;
        const TBaseMap& b = other;
        return a <=> b;
    }
};

}
