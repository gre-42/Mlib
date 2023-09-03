#pragma once
#include <Mlib/Array/Base_Dense_Array.hpp>
#include <cstddef>

namespace Mlib {

template <class TDerived, class TData, size_t... n>
class BaseDenseFixedArray: public BaseDenseArray<TDerived, TData> {};

}
