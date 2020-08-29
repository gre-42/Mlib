#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <class TData>
FixedArray<TData, 3, 3> cross(const FixedArray<TData, 3>& k) {
    return FixedArray<TData, 3, 3>{
        0, -k(2), k(1),
        k(2), 0, -k(0),
        -k(1), k(0), 0};
}

template <class TData>
FixedArray<TData, 3> cross(const FixedArray<TData, 3>& a, const FixedArray<TData, 3>& b) {
    return FixedArray<TData, 3>{
        -a(2) * b(1) + a(1) * b(2),
        a(2) * b(0) -a(0) * b(2),
        -a(1) * b(0) + a(0) * b(1)};
}

}
