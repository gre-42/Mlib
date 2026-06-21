#pragma once
#include <Mlib/Type_Traits/Map.hpp>
#include <Mlib/Type_Traits/Unordered_Map.hpp>

namespace Mlib {

template <typename T>
concept AnyStdMap = StdMap<T> || StdUnorderedMap<T>;

}
