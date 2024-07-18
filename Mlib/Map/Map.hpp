#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>
#include <stdexcept>
#include <vector>

namespace Mlib {

template <class TKey, class TValue>
class Map: public std::map<TKey, TValue> {
public:
    Map() {}
    Map(std::initializer_list<typename std::map<TKey, TValue>::value_type> init)
        : std::map<TKey, TValue>(init)
    {}

    bool contains(const TKey& key) const {
        const std::map<TKey, TValue>* m = this;
        return m->contains(key);
    }

    bool contains(const TKey& key, const TValue& value, bool deflt = false) const {
        auto it = this->find(key);
        if (it == this->end()) {
            return deflt;
        } else {
            return it->second == value;
        }
    }

    const TValue& get(const TKey& key) const {
        return const_cast<Map*>(this)->get(key);
    }

    TValue& get(const TKey& key) {
        auto it = this->find(key);
        if (it == this->end()) {
            THROW_OR_ABORT("Could not find entry with key \"" + key + '"');
        }
        return it->second;
    }

    TValue get(const TKey& key, const TValue& deflt) const {
        auto it = this->find(key);
        if (it == this->end()) {
            return deflt;
        }
        return it->second;
    }

    const TValue* try_get(const TKey& key) const {
        auto it = this->find(key);
        if (it == this->end()) {
            return nullptr;
        }
        return &it->second;
    }

    template< class... Args >
    void add(const TKey& key, Args&&... args) {
        if (!this->try_emplace(key, std::forward<Args>(args)...).second) {
            THROW_OR_ABORT("Could not insert into map");
        }
    }

    std::vector<TKey> keys() const {
        std::vector<TKey> result;
        result.reserve(this->size());
        for (const auto& [k, _] : *this) {
            result.push_back(k);
        }
        return result;
    }
};

}
