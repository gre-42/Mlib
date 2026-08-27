#pragma once
#include <Mlib/Os/Os.hpp>
#include <cstddef>
#include <list>
#include <map>
#include <stdexcept>
#include <unordered_map>

namespace Mlib {

template <class TKey, class TValue>
struct ReferenceMapElement {
    const TKey& key;
    TValue& value;
};

template <class TValue, class TListIterator>
struct ValueAndListIterator {
    template <class... Args>
    explicit ValueAndListIterator(Args&&... v)
        : value(std::forward<Args>(v)...)
    {}
    using Value = TValue;
    TValue value;
    TListIterator list_iterator;
};

template <class TList, class TMap>
class OrderedGenericMap {
public:
    using key_type = TMap::key_type;
    using value_type = TMap::value_type;    
    using mapped_type = TMap::mapped_type;
    using node_type = TMap::node_type;
    using iterator = TList::iterator;
    using iterated_type = TList::value_type;
    using payload_type = mapped_type::Value;
    OrderedGenericMap() = default;
    template <class... Args>
    explicit OrderedGenericMap(Args&&... v)
        : map_(std::forward<Args>(v)...)
    {
        if (!map_.empty()) {
            throw std::runtime_error("Ordered map not empty upon initialization");
        }
    }
    template <class... Args>
    std::pair<payload_type&, bool> try_emplace(const key_type& key, Args&&... args) {
        auto res = map_.try_emplace(key, std::forward<Args>(args)...);
        if (res.second) {
            auto list_iterator = elements_.emplace(elements_.end(), res.first->first, res.first->second.value);
            res.first->second.list_iterator = list_iterator;
        }
        return {res.first->second.value, res.second};
    }
    template <class TKey, class... Args>
    std::pair<payload_type&, bool> try_emplace(TKey&& key, Args&&... args) {
        auto res = map_.try_emplace(std::forward<TKey>(key), std::forward<Args>(args)...);
        if (res.second) {
            auto list_iterator = elements_.emplace(elements_.end(), res.first->first, res.first->second.value);
            res.first->second.list_iterator = list_iterator;
        }
        return {res.first->second.value, res.second};
    }
    decltype(auto) insert(node_type&& nh) {
        auto res = map_.insert(std::move(nh));
        if (res.inserted) {
            auto list_iterator = elements_.emplace(elements_.end(), res.position->first, res.position->second.value);
            res.position->second.list_iterator = list_iterator;
        }
        return res;
    }
    payload_type& at(const key_type& k) {
        return map_.at(k).value;
    }
    node_type extract(const key_type& k) {
        auto res = map_.extract(k);
        if (!res.empty()) {
            elements_.erase(res.mapped().list_iterator);
            res.mapped().list_iterator = elements_.end();
        } else {
            verbose_abort("OrderedGenericMap::extract failed, please use try_extract");
        }
        return res;
    }
    node_type try_extract(const key_type& k) {
        auto res = map_.try_extract(k);
        if (!res.empty()) {
            elements_.erase(res.mapped().list_iterator);
            res.mapped().list_iterator = elements_.end();
        }
        return res;
    }
    size_t erase(const key_type& k) {
        auto res = try_extract(k);
        return !res.empty();
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

template <
    class TKey,
    class TValue,
    template <class...> class TMapContainer
>
struct OrderedBinaryMapBuilder {
    using List = std::list<ReferenceMapElement<TKey, TValue>>;
    using ValueAndIterator = ValueAndListIterator<TValue, typename List::iterator>;
    using Map = TMapContainer<TKey, ValueAndIterator>;
    using OrderedMap = OrderedGenericMap<List, Map>;
};

template <
    class TValue,
    template <class...> class TMapContainer
>
struct OrderedUnaryMapBuilder {
    using AuxiliaryMap = TMapContainer<TValue>;
    using Key = AuxiliaryMap::key_type;
    using List = std::list<ReferenceMapElement<Key, TValue>>;
    using ValueAndIterator = ValueAndListIterator<TValue, typename List::iterator>;
    using Map = TMapContainer<ValueAndIterator>;
    using OrderedMap = OrderedGenericMap<List, Map>;
};

template <class TKey, class TValue>
using OrderedUnorderedMap = OrderedBinaryMapBuilder<TKey, TValue, std::unordered_map>::OrderedMap;

template <class TKey, class TValue>
using OrderedStandardMap = OrderedBinaryMapBuilder<TKey, TValue, std::map>::OrderedMap;

template <class TValue, template <class...> class TMapContainer>
using OrderedUnaryMap = OrderedUnaryMapBuilder<TValue, TMapContainer>::OrderedMap;

}
