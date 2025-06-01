#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Source_Location.hpp>
#include <map>

namespace Mlib {

template <class T>
class DanglingMap {
public:
    template <class TKey>
    decltype(auto) try_emplace(const TKey& key, SourceLocation loc) {
        auto res = elements_.try_emplace(key, key->on_destroy, loc);
        if (!res.second) {
            return res;
        }
        res.first->second.add([this, it=res.first](){ elements_.erase(it); }, loc);
        return res;
    }
    template <class TKey>
    decltype(auto) erase(const TKey& v) {
        return elements_.erase(v);
    }
    decltype(auto) begin() { return elements_.begin(); }
    decltype(auto) end() { return elements_.end(); }
    decltype(auto) begin() const { return elements_.begin(); }
    decltype(auto) end() const { return elements_.end(); }
private:
    std::map<T, DestructionFunctionsRemovalTokens> elements_;
};

}
