#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Ref.hpp>
#include <Mlib/Source_Location.hpp>
#include <vector>

namespace Mlib {

template <class T>
class DanglingBaseClassRef;

template <class TMap>
class DanglingValueGenericMap {
public:
    using key_type = TMap::key_type;
    using mapped_type = TMap::mapped_type;
    using value_type = TMap::value_type;
    using iterator = TMap::iterator;
    using const_iterator = TMap::const_iterator;
    using base_mapped_type = mapped_type::value_type;
    DanglingValueGenericMap() = default;
    DanglingValueGenericMap(const DanglingValueGenericMap&) = default;
    ~DanglingValueGenericMap() = default;
    decltype(auto) emplace(const key_type& key, const DanglingBaseClassRef<base_mapped_type>& element, SourceLocation loc) {
        auto it = elements_.try_emplace(key, element, element->on_destroy, loc);
        if (it.second) {
            it.first->second.on_destroy([this, it=it.first](){ elements_.extract(it); }, loc);
        }
        return it;
    }
    template <class TElement>
    decltype(auto) erase(const TElement& v) {
        return elements_.erase(v);
    }
    decltype(auto) begin() { return elements_.begin(); }
    decltype(auto) end() { return elements_.end(); }
    decltype(auto) begin() const { return elements_.begin(); }
    decltype(auto) end() const { return elements_.end(); }
    bool empty() const {
        return elements_.empty();
    }
    std::size_t size() const {
        return elements_.size();
    }
    template <class TKey2>
    bool contains(const TKey2& key) const {
        return elements_.contains(key);
    }
    template <class TKey2>
    decltype(auto) find(const TKey2& key) {
        return elements_.find(key);
    }
    template <class TKey2>
    decltype(auto) find(const TKey2& key) const {
        return elements_.find(key);
    }
private:
    TMap elements_;
};

}
