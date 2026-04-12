#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Ptr.hpp>
#include <Mlib/Misc/Source_Location.hpp>
#include <set>
#include <unordered_set>
#include <vector>

namespace Mlib {

template <class T>
class DanglingBaseClassPtr;

template <class T, class TElements>
class DanglingGenericSet {
public:
    DanglingGenericSet() = default;
    DanglingGenericSet(const DanglingGenericSet&) = default;
    ~DanglingGenericSet() = default;
    DanglingGenericSet(const std::vector<T>& vec, SourceLocation loc) {
        for (const auto& element : vec) {
            emplace(element, loc);
        }
    }
    decltype(auto) emplace(const DanglingBaseClassPtr<T>& element, SourceLocation loc) {
        auto it = elements_.emplace(element, element->on_destroy.deflt, loc);
        if (it.second) {
            it.first->on_destroy([this, it=it.first](){ [[maybe_unused]] elements_.extract(it); }, loc);
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
    TElements elements_;
};

template <class T>
using DanglingUnorderedSet = DanglingGenericSet<
    T,
    std::unordered_set<
        DestructionFunctionsTokensPtr<T>,
        std::hash<DestructionFunctionsTokensPtr<T>>,
        DestructionFunctionsTokensPtrComparator<T>>>;

template <class T>
using DanglingSet = DanglingGenericSet<
    T,
    std::set<
        DestructionFunctionsTokensPtr<T>,
        DestructionFunctionsTokensPtrComparator<T>>>;

}
