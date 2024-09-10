#pragma once
#include <Mlib/Map/Threadsafe_String_With_Hash_Generic_Map.hpp>
#include <unordered_map>

namespace Mlib {

template <class TValue>
using ThreadsafeStringWithHashUnorderedMap = ThreadsafeStringWithHashGenericMap<std::unordered_map<VariableAndHash<std::string>, TValue>>;

}
