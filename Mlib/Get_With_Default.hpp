#pragma once
#include <map>

namespace Mlib {

template <class K, class V>
V get_with_default(
    const std::map<K, V>& m,
    const K& k,
    const V& deflt)
{
    auto it = m.find(k);
    if (it != m.end()) {
        return it->second;
    } else {
        return deflt;
    }
}

}
