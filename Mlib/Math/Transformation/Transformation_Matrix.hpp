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
        TransformationMatrix result = uninitialized;
        invert_t_R(t, R, result.t, result.R);
        return result;
    }

    inline TransformationMatrix(Uninitialized)
        : R{ uninitialized }
        , t{ uninitialized }
    {}

    inline TransformationMatrix(const FixedArray<TDir, n, n>& R, const FixedArray<TPos, n>& t)
        : R{ R }
        , t{ t }
    {}

    inline explicit TransformationMatrix(const FixedArray<TPos, n + 1, n + 1>& m)
        : R{ R_from_NxN(m).template casted<TDir>() }
        , t{ t_from_NxN(m) }
    {}

    inline explicit TransformationMatrix(const FixedArray<TPos, n, n + 1>& m)
        : R{ R_from_NxN1(m).template casted<TDir>() }
        , t{ t_from_NxN1(m) }
    {}

    template <class TPos2, size_t... tshape>
    inline auto transform(const FixedArray<TPos2, tshape...>& rhs) const
    {
        using ResultR = decltype(TDir() * TPos2());
        using ResultT = decltype(ResultR() + TPos());
        auto R_c = R.template casted<ResultR>();
        auto rhs_c = rhs.template casted<ResultR>();
        auto t_c = t.template casted<ResultT>();
        auto x = outer(rhs_c, R_c).template casted<ResultT>();
        for (auto& r : x.rows_as_1D().row_iterable()) {
            r += t_c;
        }
        return x;
    }

    template <class TPos2, size_t... tshape>
    inline auto itransform(const FixedArray<TPos2, tshape...>& rhs) const
    {
        using ResultT = decltype(TPos2() - TPos());
        using ResultR = decltype(ResultT() * TDir());
        auto rhs_c = rhs.template casted<ResultT>();
        auto t_c = t.template casted<ResultT>();
        auto R_c = R.template casted<ResultR>();
        auto x = rhs_c;
        for (auto& r : x.rows_as_1D().row_iterable()) {
            r -= t_c;
        }
        return dot(x.template casted<ResultR>(), R_c);
    }

    inline TransformationMatrix operator * (const TransformationMatrix& rhs) const {
        return TransformationMatrix{
            dot2d(R, rhs.R),
            transform(rhs.t)};
    }

    template <class TDir2, size_t... tshape>
    inline auto rotate(const FixedArray<TDir2, tshape...>& rhs) const {
        using Result = decltype(TDir() * TDir2());
        auto R_c = R.template casted<Result>();
        auto rhs_c = rhs.template casted<Result>();
        return outer(rhs_c, R_c);
    }

    template <class TDir2, size_t... tshape>
    inline auto irotate(const FixedArray<TDir2, tshape...>& rhs) const {
        using Result = decltype(TDir() * TDir2());
        auto rhs_c = rhs.template casted<Result>();
        auto R_c = R.template casted<Result>();
        return dot(rhs_c, R_c);
    }

    template <size_t m>
    inline FixedArray<TPos, n + 1, m> project(const FixedArray<TPos, n + 1, m>& rhs) const {
        FixedArray<TPos, n + 1, m> res = uninitialized;
        res.template row_range<0, n>() = dot2d(semi_affine(), rhs);
        res[n] = rhs[n];
        return res;
    }

    inline const FixedArray<TPos, n+1, n+1> affine() const {
        return assemble_homogeneous_NxN(R.template casted<TPos>(), t);
    }

    inline const FixedArray<TPos, n, n + 1> semi_affine() const {
        return assemble_homogeneous_NxN1(R.template casted<TPos>(), t);
    }

    inline TransformationMatrix inverted() const {
        return inverse(R, t);
    }

    inline float get_scale2() const {
        return sum(squared(R)) / n;
    }

    inline float get_scale() const {
        return std::sqrt(get_scale2());
    }

    inline TransformationMatrix inverted_scaled() const {
        return inverse(R / get_scale2(), t);
    }
    
    void pre_scale(const TPos& f) {
        R *= f;
        t *= f;
    }

    TransformationMatrix pre_scaled(const TPos& f) const {
        return TransformationMatrix{R * f, t * f};
    }

    template <class TResultDir, class TResultPos>
    TransformationMatrix<TResultDir, TResultPos, n> casted() const {
        return TransformationMatrix<TResultDir, TResultPos, n>{
            R.template casted<TResultDir>(),
            t.template casted<TResultPos>()};
    }

    template <class Archive>
    void serialize(Archive& archive) {
        archive(R);
        archive(t);
    }
    FixedArray<TDir, n, n> R;
    FixedArray<TPos, n> t;
};

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
