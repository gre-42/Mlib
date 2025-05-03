#pragma once
#include <Mlib/Array/Base_Dense_Array.hpp>

namespace Mlib {

template <class TDerived, class TData>
auto floor(const BaseDenseArray<TDerived, TData>& a) {
    return a->applied([](const TData& v){ return std::floor(v); });
}

}
