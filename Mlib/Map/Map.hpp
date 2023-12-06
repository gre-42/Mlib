#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>
#include <optional>
#include <stdexcept>

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

    const std::optional<const TValue> try_get(const TKey& key) const {
        auto it = this->find(key);
        if (it == this->end()) {
            return std::nullopt;
        }
        return it->second;
    }
};

}
