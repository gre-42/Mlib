#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Ptr.hpp>
#include <Mlib/Source_Location.hpp>
#include <optional>
#include <unordered_set>
#include <vector>

namespace Mlib {

template <class T>
class DanglingBaseClassPtr;

template <class T>
class DanglingUnorderedSet {
public:
    using Element = DestructionFunctionsTokensPtr<T>;
    using Elements = std::unordered_set<Element, std::hash<Element>, DestructionFunctionsTokensPtrComparator<T>>;
    DanglingUnorderedSet() = default;
    DanglingUnorderedSet(const DanglingUnorderedSet&) = default;
    ~DanglingUnorderedSet() = default;
    DanglingUnorderedSet(const std::vector<T>& vec, SourceLocation loc) {
        for (const auto& element : vec) {
            emplace(element, loc);
        }
    }
    decltype(auto) emplace(const DanglingBaseClassPtr<T>& element, SourceLocation loc) {
        auto it = elements_.emplace(element, element->on_destroy, loc);
        if (it.second) {
            it.first->on_destroy([this, it=it.first](){ elements_.extract(it); }, loc);
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
    template <class TKey>
    bool contains(const TKey& key) const {
        return elements_.contains(key);
    }
private:
    Elements elements_;
};

}
