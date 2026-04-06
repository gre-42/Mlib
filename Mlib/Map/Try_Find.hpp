#pragma once
#include <type_traits>

namespace Mlib {

template <class TMap, class TKey>
decltype(&std::add_const_t<TMap>().find(TKey())->second)
    try_find(const TMap& m, const TKey& k)
{
    auto it = m.find(k);
    if (it == m.end()) {
        return nullptr;
    }
    return &it->second;
}

template <class TMap, class TKey>
decltype(&TMap().find(TKey())->second)
    try_find(TMap& m, const TKey& k)
{
    auto it = m.find(k);
    if (it == m.end()) {
        return nullptr;
    }
    return &it->second;
}

}
