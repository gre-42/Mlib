#pragma once
#include <Mlib/Os/Os.hpp>
#include <cstddef>
#include <list>
#include <map>
#include <unordered_map>

namespace Mlib {

template <class TKey, class TValue>
struct ReferenceMapElement {
    const TKey& key;
    TValue& value;
};

template <class TList, class TMap>
class OrderedMap {
public:
    using key_type = TMap::key_type;
    using value_type = TMap::value_type;    
    using mapped_type = TMap::mapped_type;
    using iterator = TList::iterator;
    OrderedMap() = default;
    OrderedMap(TMap map)
        : map_{ std::move(map) }
    {}
    template <class... Args>
    decltype(auto) try_emplace(const key_type& key, Args&&... args) {
        auto res = map_.try_emplace(key, std::forward<Args>(args)...);
        if (!res.second) {
            return res;
        }
        elements_.emplace_back(res.first->first, res.first->second);
        return res;
    }
    template <class TKey, class... Args>
    decltype(auto) try_emplace(TKey&& key, Args&&... args) {
        auto res = map_.try_emplace(std::forward<TKey>(key), std::forward<Args>(args)...);
        if (!res.second) {
            return res;
        }
        elements_.emplace_back(res.first->first, res.first->second);
        return res;
    }
    decltype(auto) at(const key_type& k) {
        return map_.at(k);
    }
    decltype(auto) begin() const {
        return elements_.begin();
    }
    decltype(auto) end() const {
        return elements_.end();
    }
    size_t size() const {
        return map_.size();
    }
    bool empty() const {
        return map_.empty();
    }
    void clear() {
        map_.clear();
        elements_.clear();
    }
private:
    TMap map_;
    TList elements_;
};

template <class TKey, class TValue>
using OrderedUnorderedMap = OrderedMap<std::list<ReferenceMapElement<TKey, TValue>>, std::unordered_map<TKey, TValue>>;

template <class TKey, class TValue>
using OrderedStandardMap = OrderedMap<std::list<ReferenceMapElement<TKey, TValue>>, std::map<TKey, TValue>>;

}
