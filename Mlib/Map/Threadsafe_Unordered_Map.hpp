#pragma once
#include <Mlib/Map/Threadsafe_Generic_Map.hpp>
#include <unordered_map>

namespace Mlib {

template <class TKey, class TValue>
using ThreadsafeUnorderedMap = ThreadsafeGenericMap<std::unordered_map<TKey, TValue>>;

}
