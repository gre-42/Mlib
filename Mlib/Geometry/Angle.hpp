#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TData>
TData normalized_radians(const TData& a) {
    return std::remainderf(a, (TData)(2 * M_PI));
}

template <typename TData, size_t... tshape>
FixedArray<TData, tshape...> normalized_radians(const FixedArray<TData, tshape...>& a) {
    return a.applied([](const TData& v){ return normalized_radians(v);});
}

}
