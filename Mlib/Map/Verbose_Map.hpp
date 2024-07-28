#pragma once
#include <Mlib/Map/Verbose_Generic_Map.hpp>
#include <map>

namespace Mlib {

template <class TKey, class TValue>
using VerboseMap = VerboseGenericMap<std::map<TKey, TValue>>;

}
