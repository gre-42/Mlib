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

    FixedArray<TData, 3, 3> minv; // inverse of matrix m
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
    TData det = -m(0u, 0u) * squared(m(0u, 1u)) + squared(m(0u, 0u)) * m(1u, 1u)
              - squared(m(0u, 2u)) * m(1u, 1u) + 2 * m(0u, 1u) * m(0u, 2u) * m(1u, 2u)
              - m(0u, 0u) * squared(m(1u, 2u));

    TData di = 1 / det;

    FixedArray<TData, 3, 3> a;
    a(0u, 0u) = (m(0u, 0u) * m(1u, 1u) - squared(m(1u, 2u))) * di;
    a(0u, 1u) = (m(0u, 2u) * m(1u, 2u) - m(0u, 0u) * m(0u, 1u)) * di;
    a(1u, 0u) = a(0u, 1u);
    a(0u, 2u) = (m(0u, 1u) * m(1u, 2u) - m(0u, 2u) * m(1u, 1u)) * di;
    a(2u, 0u) = a(0u, 2u);

    a(1u, 1u) = (squared(m(0u, 0u)) - squared(m(0u, 2u))) * di;
    a(1u, 2u) = (m(0u, 1u) * m(0u, 2u) - m(0u, 0u) * m(1u, 2u)) * di;
    a(2u, 1u) = a(1u, 2u);

    a(2u, 2u) = (m(0u, 0u) * m(1u, 1u) - squared(m(0u, 1u))) * di;
    return a;
}

}
