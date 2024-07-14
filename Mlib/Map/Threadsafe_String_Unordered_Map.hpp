#pragma once
#include <Mlib/Map/Threadsafe_String_Generic_Map.hpp>
#include <unordered_map>

namespace Mlib {

template <class TValue>
using ThreadsafeStringUnorderedMap = ThreadsafeStringGenericMap<std::unordered_map<std::string, TValue>>;

}
