#pragma once
#include <Mlib/Map/String_With_Hash_Generic_Map.hpp>
#include <unordered_map>

namespace Mlib {

template <class TValue>
using StringWithHashUnorderedMap = StringWithHashGenericMap<std::unordered_map<VariableAndHash<std::string>, TValue>>;

}
