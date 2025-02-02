#pragma once
#include <Mlib/Map/Verbose_Generic_Map.hpp>
#include <unordered_map>

namespace Mlib {

template <class TKey, class TValue>
using VerboseUnorderedMap = VerboseGenericMap<std::unordered_map<TKey, TValue>>;

}
