#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

/**
 * From: https://stackoverflow.com/questions/983999/simple-3x3-matrix-inverse-code-c
 */
template <class TData>
FixedArray<TData, 3, 3> fixed_inverse_3x3(const FixedArray<TData, 3, 3>& m) {
    // computes the inverse of a matrix m
    TData det = m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) -
                m(0, 1) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0)) +
                m(0, 2) * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0));

    TData invdet = 1 / det;

    UFixedArray<TData, 3, 3> minv; // inverse of matrix m
    minv(0, 0) = (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) * invdet;
    minv(0, 1) = (m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2)) * invdet;
    minv(0, 2) = (m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1)) * invdet;
    minv(1, 0) = (m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2)) * invdet;
    minv(1, 1) = (m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0)) * invdet;
    minv(1, 2) = (m(1, 0) * m(0, 2) - m(0, 0) * m(1, 2)) * invdet;
    minv(2, 0) = (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)) * invdet;
    minv(2, 1) = (m(2, 0) * m(0, 1) - m(0, 0) * m(2, 1)) * invdet;
    minv(2, 2) = (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) * invdet;
    return minv;
}

/**
 * From: https://gamedev.net/forums/topic/380143-inverse-of-symmetric-3x3-matrix/3510626/
 */
template <class TData>
FixedArray<TData, 3, 3> fixed_symmetric_inverse_3x3(const FixedArray<TData, 3, 3>& m) {
    TData det = -m(0, 0) * squared(m(0, 1)) + squared(m(0, 0)) * m(1, 1)
              - squared(m(0, 2)) * m(1, 1) + 2 * m(0, 1) * m(0, 2) * m(1, 2)
              - m(0, 0) * squared(m(1, 2));

    TData di = 1 / det;

    FixedArray<TData, 3, 3> a = uninitialized;
    a(0, 0) = (m(0, 0) * m(1, 1) - squared(m(1, 2))) * di;
    a(0, 1) = (m(0, 2) * m(1, 2) - m(0, 0) * m(0, 1)) * di;
    a(1, 0) = a(0, 1);
    a(0, 2) = (m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1)) * di;
    a(2, 0) = a(0, 2);

    a(1, 1) = (squared(m(0, 0)) - squared(m(0, 2))) * di;
    a(1, 2) = (m(0, 1) * m(0, 2) - m(0, 0) * m(1, 2)) * di;
    a(2, 1) = a(1, 2);

    a(2, 2) = (m(0, 0) * m(1, 1) - squared(m(0, 1))) * di;
    return a;
}

}
