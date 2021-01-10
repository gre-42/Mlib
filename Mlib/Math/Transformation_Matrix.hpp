#pragma once
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <cmath>
#include <iostream>

namespace Mlib {

template <class TData>
class TransformationMatrix {
public:
    static TransformationMatrix identity() {
        return TransformationMatrix{
            fixed_identity_array<TData, 3>(),
            fixed_zeros<TData, 3>()};
    }

    static TransformationMatrix inverse(const FixedArray<TData, 3, 3>& R, const FixedArray<float, 3>& t) {
        TransformationMatrix result;
        invert_t_R(t, R, result.t_, result.R_);
        return result;
    }

    TransformationMatrix()
    {}

    TransformationMatrix(const FixedArray<TData, 3, 3>& R, const FixedArray<float, 3>& t)
    : R_{R},
      t_{t}
    {}

    explicit TransformationMatrix(const FixedArray<TData, 4, 4>& m)
    : R_{R3_from_4x4(m)},
      t_{t3_from_4x4(m)}
    {}

    FixedArray<TData, 3> operator * (const FixedArray<TData, 3>& rhs) const {
        return dot1d(R_, rhs) + t_;
    }

    TransformationMatrix operator * (const TransformationMatrix& rhs) const {
        return TransformationMatrix{
            dot2d(R_, rhs.R_),
            dot1d(R_, rhs.t_) + t_};
    }

    FixedArray<TData, 3> rotate(const FixedArray<TData, 3>& rhs) const {
        return dot1d(R_, rhs);
    }

    const FixedArray<TData, 3, 3>& R() const {
        return R_;
    }

    const FixedArray<TData, 3>& t() const {
        return t_;
    }

    FixedArray<TData, 3, 3>& R() {
        return R_;
    }

    FixedArray<TData, 3>& t() {
        return t_;
    }

    const FixedArray<TData, 4, 4> affine() const {
        return assemble_homogeneous_4x4(R_, t_);
    }

    TransformationMatrix inverted() const {
        return inverse(R_, t_);
    }

    TransformationMatrix inverted_scaled() const {
        auto scale2 = sum(squared(R_)) / 3;
        return inverse(R_ / scale2, t_);
    }
private:
    FixedArray<TData, 3, 3> R_;
    FixedArray<TData, 3> t_;
};

}
