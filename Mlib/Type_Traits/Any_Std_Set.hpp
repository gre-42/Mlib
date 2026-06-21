#pragma once
#include <Mlib/Type_Traits/Set.hpp>
#include <Mlib/Type_Traits/Unordered_Set.hpp>

namespace Mlib {

template <typename T>
concept AnyStdSet = StdSet<T> || StdUnorderedSet<T>;

}
