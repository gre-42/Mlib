#pragma once
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <cmath>
#include <iostream>

#pragma GCC push_options
#pragma GCC optimize ("O3")

namespace Mlib {

template <class TData>
class TransformationMatrix {
public:
    static inline TransformationMatrix identity() {
        return TransformationMatrix{
            fixed_identity_array<TData, 3>(),
            fixed_zeros<TData, 3>()};
    }

    static inline TransformationMatrix inverse(const FixedArray<TData, 3, 3>& R, const FixedArray<float, 3>& t) {
        TransformationMatrix result;
        invert_t_R(t, R, result.t_, result.R_);
        return result;
    }

    inline TransformationMatrix()
    {}

    inline TransformationMatrix(const FixedArray<TData, 3, 3>& R, const FixedArray<float, 3>& t)
    : R_{R},
      t_{t}
    {}

    inline explicit TransformationMatrix(const FixedArray<TData, 4, 4>& m)
    : R_{R3_from_4x4(m)},
      t_{t3_from_4x4(m)}
    {}

    inline FixedArray<TData, 3> operator * (const FixedArray<TData, 3>& rhs) const {
        return dot1d(R_, rhs) + t_;
    }

    inline TransformationMatrix operator * (const TransformationMatrix& rhs) const {
        return TransformationMatrix{
            dot2d(R_, rhs.R_),
            dot1d(R_, rhs.t_) + t_};
    }

    inline FixedArray<TData, 3> rotate(const FixedArray<TData, 3>& rhs) const {
        return dot1d(R_, rhs);
    }

    inline const FixedArray<TData, 3, 3>& R() const {
        return R_;
    }

    inline const FixedArray<TData, 3>& t() const {
        return t_;
    }

    inline FixedArray<TData, 3, 3>& R() {
        return R_;
    }

    inline FixedArray<TData, 3>& t() {
        return t_;
    }

    inline const FixedArray<TData, 4, 4> affine() const {
        return assemble_homogeneous_4x4(R_, t_);
    }

    inline TransformationMatrix inverted() const {
        return inverse(R_, t_);
    }

    inline TransformationMatrix inverted_scaled() const {
        auto scale2 = sum(squared(R_)) / 3;
        return inverse(R_ / scale2, t_);
    }
private:
    FixedArray<TData, 3, 3> R_;
    FixedArray<TData, 3> t_;
};

}

#pragma GCC pop_options
