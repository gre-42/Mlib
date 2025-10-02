#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Ref.hpp>
#include <Mlib/Source_Location.hpp>
#include <list>

namespace Mlib {

template <class T>
class DanglingList {
public:
    template <class TElement>
    decltype(auto) emplace_back(const TElement& element, SourceLocation loc) {
        auto it = elements_.emplace(elements_.end(), element, element->on_destroy, loc);
        it->on_destroy([this, it](){
            std::list<DestructionFunctionsTokensRef<T>> tmp;
            tmp.splice(tmp.end(), elements_, it);
        }, loc);
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
    decltype(auto) front() {
        return elements_.front();
    }
    decltype(auto) back() {
        return elements_.back();
    }
private:
    std::list<DestructionFunctionsTokensRef<T>> elements_;
};

}
