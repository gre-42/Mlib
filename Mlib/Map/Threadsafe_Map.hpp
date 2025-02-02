#pragma once
#include <Mlib/Map/Threadsafe_Generic_Map.hpp>
#include <map>

namespace Mlib {

template <class TKey, class TValue>
using ThreadsafeMap = ThreadsafeGenericMap<std::map<TKey, TValue>>;

}
