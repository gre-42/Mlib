#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Geometry/Cross.hpp>

namespace Mlib{

template <class TData>
Array<TData> rodrigues(const Array<TData>& k, const TData& theta);

template <class TData>
Array<TData> tait_bryan_angles_2_matrix(const Array<TData>& angles);

template <class TData>
Array<TData> rodrigues(const Array<TData>& k) {
    assert(all(k.shape() == ArrayShape{3}));
    if (sum(squared(k)) < 1e-12) {
        static const Array<TData> I = identity_array<TData>(3);
        return I;
    } else {
        TData len_k = std::sqrt(sum(norm(k)));
        return rodrigues(k / len_k, len_k);
    }
}

template <class TData>
Array<TData> rodrigues(const Array<TData>& k, const TData& theta) {
    assert(all(k.shape() == ArrayShape{3}));

    // slow
    // Array<TData> K{cross(k)};
    // static const Array<TData> I = identity_array<TData>(3);
    // return I + std::sin(theta) * K + (1 - std::cos(theta)) * dot(K, K);

    static thread_local Array<TData> K = zeros<TData>(ArrayShape{3, 3});
    K(0, 1) = -k(2);
    K(0, 2) = k(1);
    K(1, 0) = k(2);
    K(1, 2) = -k(0);
    K(2, 0) = -k(1);
    K(2, 1) = k(0);
    Array<TData> result{ArrayShape{3, 3}};
    identity_array(result);
    result += std::sin(theta) * K;
    result += (1 - std::cos(theta)) * dot(K, K);
    return result;
}

template <class TData>
Array<TData> tait_bryan_angles_2_matrix(const Array<TData>& angles)
{

    // slow
    // Array<TData> I = identity_array<TData>(3);
    // Array<TData> result = rodrigues(I[0], alpha);
    // result = (result, rodrigues(I[1], beta).T());
    // result = (result, rodrigues(I[2], gamma).T());
    // return result;

    assert(angles.length() == 3);
    static const Array<TData> I = identity_array<TData>(3);
    static const Array<TData> I0 = I[0];
    static const Array<TData> I1 = I[1];
    static const Array<TData> I2 = I[2];
    static thread_local Array<TData> tmp{ArrayShape{3, 3}};
    Array<TData> ro = rodrigues(I0, angles(0));
    dot2d(rodrigues(I1, angles(1)), ro, tmp);
    dot2d(rodrigues(I2, angles(2)), tmp, ro);
    return ro;
}

/**
 * Source: learnopencv.com
 * Modification: Transposed the matrix and corrected the signs.
 */
template <class TData>
Array<TData> matrix_2_tait_bryan_angles(Array<TData> R, bool force_singular = false) {
    TData sy = std::sqrt(squared(R(0, 0)) + squared(R(1, 0)));
    bool singular = (sy < 1e-6);

    if (!singular && !force_singular) {
        return Array<float>{
            std::atan2(R(2, 1), R(2, 2)),
            std::atan2(-R(2, 0), sy),
            std::atan2(R(1, 0), R(0, 0))};
    } else {
        return Array<float>{
            std::atan2(-R(1, 2), R(1, 1)),
            std::atan2(-R(2, 0), sy),
            0};
    }
}

/**
 * Not working, only works around point 0.
 * Using axis-angle representation instead,
 * where only the angle varies.
 */
// template <class TData>
// Array<TData> rodrigues_jacobian_dk(const Array<TData>& k, const Array<TData>& x) {
//     auto R = rodrigues(k);
//     auto K = cross(-dot(R.T(), x));
//     return dot(R, (K, R));
// }

template <class TData>
Array<TData> rodrigues_gradient_dtheta(
    const Array<TData>& k,
    const TData& theta,
    const Array<TData>& x)
{
    auto R = rodrigues(k, theta);
    auto v = cross(k, x);
    return dot(R, v);
}

template <class TData>
Array<TData> tait_bryan_angles_dtheta(
    const Array<TData>& theta,
    const Array<TData>& x)
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
    static const Array<TData> I = identity_array<TData>(3);
    // Changed order to be compatible with the rodrigues-implementation
    static const Array<TData> I0 = I[0];
    static const Array<TData> I1 = I[1];
    static const Array<TData> I2 = I[2];
    Array<TData> R0 = rodrigues(I0, theta(0));
    Array<TData> R1 = rodrigues(I1, theta(1));
    Array<TData> R2 = rodrigues(I2, theta(2));

    Array<TData> r1 = dot1d(R0, x);
    Array<TData> r2 = dot1d(R1, r1);
    // Changed theta-order to be compatible with the rodrigues-implementation
    Array<TData> dy_da2 = rodrigues_gradient_dtheta(I2, theta(2), r2);
    Array<TData> dy_da1 = dot1d(R2, rodrigues_gradient_dtheta(I1, theta(1), r1));
    Array<TData> dy_da0 = dot1d(R2, dot1d(R1, rodrigues_gradient_dtheta(I0, theta(0), x)));

    // Changed order to be compatible with the rodrigues-implementation
    return Array<TData>{
        {dy_da0(0), dy_da1(0), dy_da2(0)},
        {dy_da0(1), dy_da1(1), dy_da2(1)},
        {dy_da0(2), dy_da1(2), dy_da2(2)}};
}

}
