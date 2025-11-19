#pragma once
#include <Mlib/Memory/Dangling_Value_Generic_Map.hpp>
#include <map>

namespace Mlib {

template <class TKey, class TValue>
using DanglingValueMap = DanglingValueGenericMap<std::map<TKey, DestructionFunctionsTokensRef<TValue>>>;

}
