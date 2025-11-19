#pragma once
#include <Mlib/Memory/Dangling_Value_Generic_Map.hpp>
#include <unordered_map>

namespace Mlib {

template <class TKey, class TValue>
using DanglingValueUnorderedMap = DanglingValueGenericMap<std::unordered_map<TKey, DestructionFunctionsTokensRef<TValue>>>;

}
