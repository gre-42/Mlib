#pragma once
#include <Mlib/Memory/Dangling_Key_Generic_Map.hpp>
#include <unordered_map>

namespace Mlib {

template <class TKey, class TMapped>
using DanglingKeyUnorderedMap = DanglingKeyGenericMap<std::unordered_map<DestructionFunctionsTokensPtr<TKey>, TMapped>>;

}
