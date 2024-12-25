#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

namespace Mlib {

template <class T, size_t n>
class DiagonalScaleMatrix {
public:
    explicit DiagonalScaleMatrix(const FixedArray<T, n>& d)
        : d(d)
    {}
    FixedArray<T, n> d;
};

template <class T, size_t R, size_t C>
FixedArray<T, R, C> dot2d(const FixedArray<T, R, C>& lhs, const DiagonalScaleMatrix<T, C>& d) {
    FixedArray<T, R, C> result = uninitialized;
    for (size_t r = 0; r < R; ++r) {
        for (size_t c = 0; c < C; ++c) {
            result(r, c) = lhs(r, c) * d.d(c);
        }
    }
    return result;
}

template <class T, size_t R, size_t C>
FixedArray<T, R, C> dot2d(const DiagonalScaleMatrix<T, R>& d, const FixedArray<T, R, C>& rhs) {
    FixedArray<T, R, C> result = uninitialized;
    for (size_t r = 0; r < R; ++r) {
        for (size_t c = 0; c < C; ++c) {
            result(r, c) = rhs(r, c) * d.d(r);
        }
    }
    return result;
}

template <class TDir, class TPos, size_t n>
TransformationMatrix<TDir, TPos, n> operator * (const TransformationMatrix<TDir, TPos, n>& a, const DiagonalScaleMatrix<TDir, n>& b) {
    return TransformationMatrix<TDir, TPos, n>{dot2d(a.R, b), a.t};
}

}
