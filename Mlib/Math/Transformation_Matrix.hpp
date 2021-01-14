#pragma once
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <cmath>
#include <iostream>

#pragma GCC push_options
#pragma GCC optimize ("O3")

namespace Mlib {

template <class TData, size_t n>
class TransformationMatrix {
public:
    static inline TransformationMatrix identity() {
        return TransformationMatrix{
            fixed_identity_array<TData, n>(),
            fixed_zeros<TData, n>()};
    }

    static inline TransformationMatrix inverse(const FixedArray<TData, n, n>& R, const FixedArray<float, n>& t) {
        TransformationMatrix result;
        invert_t_R(t, R, result.t_, result.R_);
        return result;
    }

    inline TransformationMatrix()
    {}

    inline TransformationMatrix(const FixedArray<TData, n, n>& R, const FixedArray<float, n>& t)
    : R_{R},
      t_{t}
    {}

    inline explicit TransformationMatrix(const FixedArray<TData, n+1, n+1>& m)
    : R_{R_from_NxN(m)},
      t_{t_from_NxN(m)}
    {}

    inline FixedArray<TData, n> operator * (const FixedArray<TData, n>& rhs) const {
        return dot1d(R_, rhs) + t_;
    }

    inline TransformationMatrix operator * (const TransformationMatrix& rhs) const {
        return TransformationMatrix{
            dot2d(R_, rhs.R_),
            dot1d(R_, rhs.t_) + t_};
    }

    inline FixedArray<TData, n> rotate(const FixedArray<TData, n>& rhs) const {
        return dot1d(R_, rhs);
    }

    inline const FixedArray<TData, n, n>& R() const {
        return R_;
    }

    inline const FixedArray<TData, n>& t() const {
        return t_;
    }

    inline FixedArray<TData, n, n>& R() {
        return R_;
    }

    inline FixedArray<TData, n>& t() {
        return t_;
    }

    inline const FixedArray<TData, n+1, n+1> affine() const {
        return assemble_homogeneous_NxN(R_, t_);
    }

    inline TransformationMatrix inverted() const {
        return inverse(R_, t_);
    }

    inline TransformationMatrix inverted_scaled() const {
        auto scale2 = sum(squared(R_)) / n;
        return inverse(R_ / scale2, t_);
    }
private:
    FixedArray<TData, n, n> R_;
    FixedArray<TData, n> t_;
};

}

#pragma GCC pop_options
