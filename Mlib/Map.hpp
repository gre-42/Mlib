#pragma once
#include <map>

namespace Mlib {

template <class TKey, class TValue>
class Map: public std::map<TKey, TValue> {
public:
    bool contains(const TKey& key) const {
        const std::map<TKey, TValue>* m = this;
        return m->contains(key);
    }

    bool contains(const TKey& key, const TValue& value) const {
        auto it = this->find(key);
        return (it != this->end()) && (it->second == value);
    }
};

}
