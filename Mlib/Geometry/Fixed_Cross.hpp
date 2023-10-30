#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Type_Traits/HigherPrecision.hpp>

namespace Mlib {

template <class TData>
FixedArray<TData, 3, 3> cross(const FixedArray<TData, 3>& k) {
    return FixedArray<TData, 3, 3>::init(
        TData{0}, -k(2), k(1),
        k(2), TData{0}, -k(0),
        -k(1), k(0), TData{0});
}

template <class TData>
FixedArray<TData, 3> cross(const FixedArray<TData, 3>& a, const FixedArray<TData, 3>& b) {
    using H = typename HigherPrecision<TData>::value_type;
    using L = TData;
    return FixedArray<TData, 3>::init(
        L(H(-a(2)) * H(b(1)) + H(a(1)) * H(b(2))),
        L(H(a(2)) * H(b(0)) + H(-a(0)) * H(b(2))),
        L(H(-a(1)) * H(b(0)) + H(a(0)) * H(b(1))));
}

}
