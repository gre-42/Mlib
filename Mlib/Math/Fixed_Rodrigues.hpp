#pragma once
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib{

template <class TData>
FixedArray<TData, 3, 3> rodrigues1(
    const FixedArray<TData, 3>& k,
    bool check_angle = true)
{
    if (sum(squared(k)) < 1e-12) {
        return fixed_identity_array<TData, 3>();
    } else {
        TData len_k = std::sqrt(sum(norm(k)));
        return rodrigues2(k / len_k, len_k, check_angle);
    }
}

template <class TData>
FixedArray<TData, 3, 3> rodrigues2(
    const FixedArray<TData, 3>& k,
    const TData& theta,
    bool check_angle = true)
{
    if (check_angle) {
        assert(std::abs(theta) < TData{2.1 * M_PI});
    }
    FixedArray<TData, 3, 3> K{cross(k)};
    FixedArray<TData, 3, 3> I = fixed_identity_array<TData, 3>();
    return I + std::sin(theta) * K + (1 - std::cos(theta)) * dot2d(K, K);
}

template <class TData>
FixedArray<TData, 3, 3> tait_bryan_angles_2_matrix(
    const FixedArray<TData, 3>& angles,
    const FixedArray<size_t, 3>& indices = {(size_t)0, (size_t)1, (size_t)2})
{
    assert(all(indices < size_t(3)));
    FixedArray<TData, 3, 3> I = fixed_identity_array<TData, 3>();
    FixedArray<TData, 3, 3> result = rodrigues2(I[indices(0)], angles(indices(0)));
    result = dot2d(rodrigues2(I[indices(1)], angles(indices(1))), result);
    result = dot2d(rodrigues2(I[indices(2)], angles(indices(2))), result);
    return result;
}

/**
 * Source: https://www.learnopencv.com/rotation-matrix-to-euler-angles/
 * Modification: Transposed the matrix and corrected the signs.
 */
template <class TData>
FixedArray<TData, 3> matrix_2_tait_bryan_angles(FixedArray<TData, 3, 3> R, bool force_singular = false) {
    TData sy = std::sqrt(squared(R(0, 0)) + squared(R(1, 0)));
    bool singular = (sy < 1e-6);

    if (!singular && !force_singular) {
        return FixedArray<TData, 3>{
            std::atan2(R(2, 1), R(2, 2)),
            std::atan2(-R(2, 0), sy),
            std::atan2(R(1, 0), R(0, 0))};
    } else {
        return FixedArray<TData, 3>{
            std::atan2(-R(1, 2), R(1, 1)),
            std::atan2(-R(2, 0), sy),
            0.f};
    }
}

/**
 * From: https://en.wikipedia.org/wiki/Axis-angle_representation#Relationship_to_other_representations
 */
template <class TData>
FixedArray<TData, 3> inverse_rodrigues(const FixedArray<TData, 3, 3>& R) {
    TData t = std::acos((R(0, 0) + R(1, 1) + R(2, 2) - 1) / 2);
    FixedArray<TData, 3> w = FixedArray<TData, 3>{
        R(2, 1) - R(1, 2),
        R(0, 2) - R(2, 0),
        R(1, 0) - R(0, 1)};
    TData len2 = sum(squared(w));
    if (len2 < (TData)1e-12) {
        return fixed_zeros<float, 3>();
    }
    w /= std::sqrt(len2);
    return w * t;
}

/**
 * From: https://math.stackexchange.com/questions/83874/efficient-and-accurate-numerical-implementation-of-the-inverse-rodrigues-rotatio
 */
template <class TData>
FixedArray<TData, 3> inverse_rodrigues2(const FixedArray<TData, 3, 3>& R) {
    FixedArray<TData, 3> x{
        R(2, 1) - R(1, 2),
        R(0, 2) - R(2, 0),
        R(1, 0) - R(0, 1)};
    TData len2 = sum(squared(x));
    if (len2 < 1e-12) {
        return fixed_zeros<TData, 3>();
    }
    x /= std::sqrt(len2);
    TData s = sign(x(2));
    TData a = -1 / (s + x(2));
    TData b = x(0) * x(1) * a;
    FixedArray<TData, 3> y{
        1 + s * a * squared(x(0)),
        s * b,
        -s * x(0)};
    FixedArray<TData, 3> z{
        b,
        s + a * squared(x(1)),
        -x(1)};
    FixedArray<TData, 3> Ry{dot1d(R, y)};
    TData t = std::atan2(
        dot0d(z, Ry),
        dot0d(y, Ry));
    return x * t;
}

/**
 * Not working, only works around point 0.
 * Using axis-angle representation instead,
 * where only the angle varies.
 */
 // template <class TData>
 // Array<TData> rodrigues_jacobian_dk(const Array<TData>& k, const Array<TData>& x) {
 //     auto R = rodrigues1(k);
 //     auto K = cross(-dot(R.T(), x));
 //     return dot(R, (K, R));
 // }

template <class TData>
FixedArray<TData, 3> rodrigues_gradient_dtheta(
    const FixedArray<TData, 3>& k,
    const TData& theta,
    const FixedArray<TData, 3>& x)
{
    auto R = rodrigues2(k, theta);
    auto v = cross(k, x);
    return dot1d(R, v);
}

template <class TData>
FixedArray<TData, 3, 3> tait_bryan_angles_dtheta(
    const FixedArray<TData, 3>& theta,
    const FixedArray<TData, 3>& x)
{
    assert(theta.length() == 3);
    assert(x.length() == 3);
    // y = R2(a2) * R1(a1) * R0(a0) * x
    //           r2       r1
    //
    // dy / da2 = (dy / da2; r2)
    // dy / da1 = (dy / dr2) * (dr2 / da1; r1) = R2 * (dr2 / da1; r1)
    // dy / da0 = (dy / dr1) * (dr1 / da0; x) = R2 * R1 * (dr1 / da0; x)
    //           (dr1 / da0; x)
    static const FixedArray<TData, 3, 3> I = fixed_identity_array<TData, 3>();
    // Changed order to be compatible with the rodrigues-implementation
    static const FixedArray<TData, 3> I0 = I[0];
    static const FixedArray<TData, 3> I1 = I[1];
    static const FixedArray<TData, 3> I2 = I[2];
    FixedArray<TData, 3, 3> R0 = rodrigues2(I0, theta(0));
    FixedArray<TData, 3, 3> R1 = rodrigues2(I1, theta(1));
    FixedArray<TData, 3, 3> R2 = rodrigues2(I2, theta(2));

    FixedArray<TData, 3> r1 = dot1d(R0, x);
    FixedArray<TData, 3> r2 = dot1d(R1, r1);
    // Changed theta-order to be compatible with the rodrigues-implementation
    FixedArray<TData, 3> dy_da2 = rodrigues_gradient_dtheta(I2, theta(2), r2);
    FixedArray<TData, 3> dy_da1 = dot1d(R2, rodrigues_gradient_dtheta(I1, theta(1), r1));
    FixedArray<TData, 3> dy_da0 = dot1d(R2, dot1d(R1, rodrigues_gradient_dtheta(I0, theta(0), x)));

    // Changed order to be compatible with the rodrigues-implementation
    return FixedArray<TData, 3, 3>::init(
        dy_da0(0), dy_da1(0), dy_da2(0),
        dy_da0(1), dy_da1(1), dy_da2(1),
        dy_da0(2), dy_da1(2), dy_da2(2));
}

}
