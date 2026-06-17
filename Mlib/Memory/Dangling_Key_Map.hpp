#pragma once
#include <Mlib/Memory/Dangling_Key_Generic_Map.hpp>
#include <map>

namespace Mlib {

template <class TKey, class TMapped>
using DanglingKeyMap = DanglingKeyGenericMap<std::map<DestructionFunctionsTokensPtr<TKey>, TMapped>>;

}
