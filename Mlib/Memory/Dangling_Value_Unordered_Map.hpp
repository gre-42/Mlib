#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Ref.hpp>
#include <Mlib/Source_Location.hpp>
#include <unordered_map>
#include <vector>

namespace Mlib {

template <class T>
class DanglingBaseClassRef;

template <class TKey, class TValue>
class DanglingValueUnorderedMap {
public:
    using Element = DestructionFunctionsTokensRef<TValue>;
    using Elements = std::unordered_map<TKey, Element>;
    using iterator = Elements::iterator;
    using const_iterator = Elements::const_iterator;
    DanglingValueUnorderedMap() = default;
    DanglingValueUnorderedMap(const DanglingValueUnorderedMap&) = default;
    ~DanglingValueUnorderedMap() = default;
    decltype(auto) emplace(const TKey& key, const DanglingBaseClassRef<TValue>& element, SourceLocation loc) {
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
    Elements elements_;
};

}
