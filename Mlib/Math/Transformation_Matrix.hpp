#pragma once
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <cmath>
#include <iostream>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix {
public:
    static inline TransformationMatrix identity() {
        return TransformationMatrix{
            fixed_identity_array<TDir, n>(),
            fixed_zeros<TPos, n>()};
    }

    static inline TransformationMatrix inverse(const FixedArray<TDir, n, n>& R, const FixedArray<TPos, n>& t) {
        TransformationMatrix result;
        invert_t_R(t, R, result.t_, result.R_);
        return result;
    }

    inline TransformationMatrix()
    {}

    inline TransformationMatrix(const FixedArray<TDir, n, n>& R, const FixedArray<TPos, n>& t)
    : R_{R},
      t_{t}
    {}

    inline explicit TransformationMatrix(const FixedArray<TPos, n+1, n+1>& m)
    : R_{R_from_NxN(m) TEMPLATEV casted<TDir>()},
      t_{t_from_NxN(m)}
    {}

    inline explicit TransformationMatrix(const FixedArray<TPos, n, n + 1>& m)
    : R_{ R_from_NxN1(m) TEMPLATEV casted<TDir>() },
      t_{ t_from_NxN1(m) }
    {}

    inline FixedArray<TPos, n> transform(const FixedArray<TPos, n>& rhs) const {
        return dot1d(R_ TEMPLATEV casted<TPos>(), rhs) + t_;
    }

    inline FixedArray<TPos, n> itransform(const FixedArray<TPos, n>& rhs) const {
        return dot(rhs - t_, R_ TEMPLATEV casted<TPos>());
    }

    inline TransformationMatrix operator * (const TransformationMatrix& rhs) const {
        return TransformationMatrix{
            dot2d(R_, rhs.R_),
            dot1d(R_ TEMPLATEV casted<TPos>(), rhs.t_) + t_};
    }

    inline FixedArray<TDir, n> rotate(const FixedArray<TDir, n>& rhs) const {
        return dot1d(R_, rhs);
    }

    inline FixedArray<TDir, n> irotate(const FixedArray<TDir, n>& rhs) const {
        return dot(rhs, R_);
    }

    template <size_t m>
    inline FixedArray<TPos, n + 1, m> project(const FixedArray<TPos, n + 1, m>& rhs) const {
        FixedArray<TPos, n + 1, m> res;
        res TEMPLATEV row_range<0, n>() = dot2d(semi_affine(), rhs);
        res[n] = rhs[n];
        return res;
    }

    inline const FixedArray<TDir, n, n>& R() const {
        return R_;
    }

    inline const FixedArray<TPos, n>& t() const {
        return t_;
    }

    inline FixedArray<TDir, n, n>& R() {
        return R_;
    }

    inline FixedArray<TPos, n>& t() {
        return t_;
    }

    inline const FixedArray<TPos, n+1, n+1> affine() const {
        return assemble_homogeneous_NxN(R_ TEMPLATEV casted<TPos>(), t_);
    }

    inline const FixedArray<TPos, n, n + 1> semi_affine() const {
        return assemble_homogeneous_NxN1(R_ TEMPLATEV casted<TPos>(), t_);
    }

    inline TransformationMatrix inverted() const {
        return inverse(R_, t_);
    }

    inline float get_scale2() const {
        return sum(squared(R_)) / n;
    }

    inline float get_scale() const {
        return std::sqrt(get_scale2());
    }

    inline TransformationMatrix inverted_scaled() const {
        return inverse(R_ / get_scale2(), t_);
    }
    
    void pre_scale(const TPos& f) {
        R_ *= f;
        t_ *= f;
    }

    TransformationMatrix pre_scaled(const TPos& f) const {
        return TransformationMatrix{R_ * f, t_ * f};
    }

    template <class TResultDir, class TResultPos>
    TransformationMatrix<TResultDir, TResultPos, n> casted() const {
        return TransformationMatrix<TResultDir, TResultPos, n>{
            R_.template casted<TResultDir>(),
            t_.template casted<TResultPos>()};
    }

    template <class Archive>
    void serialize(Archive& archive) {
        archive(R_);
        archive(t_);
    }
private:
    FixedArray<TDir, n, n> R_;
    FixedArray<TPos, n> t_;
};

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
