#pragma once
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

    bool contains(const TKey& key, const TValue& value) const {
        auto it = this->find(key);
        return (it != this->end()) && (it->second == value);
    }

    const TValue& get(const TKey& key) const {
        auto it = this->find(key);
        if (it == this->end()) {
            throw std::runtime_error("Could not find entry with key \"" + key + '"');
        }
        return it->second;
    }

    const std::optional<const TValue> try_get(const TKey& key) const {
        auto it = this->find(key);
        if (it == this->end()) {
            return nullptr;
        }
        return it->second;
    }
};

}
