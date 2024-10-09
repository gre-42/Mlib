#pragma once
#include <Mlib/Array/Base_Dense_Array.hpp>

namespace Mlib {

template <class TDerived, class TData>
auto nan_to_num(const BaseDenseArray<TDerived, TData>& a, const TData& num) {
    return a->applied([&num](const TData& v){ return std::isnan(v) ? num : v ; });
}

}
