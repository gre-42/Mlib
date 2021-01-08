#pragma once
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib{

template <class TData>
FixedArray<TData, 3, 3> rodrigues(
    const FixedArray<TData, 3>& k,
    bool check_angle = true)
{
    if (sum(squared(k)) < 1e-12) {
        return fixed_identity_array<TData, 3>();
    } else {
        TData len_k = std::sqrt(sum(norm(k)));
        return rodrigues(k / len_k, len_k, check_angle);
    }
}

template <class TData>
FixedArray<TData, 3, 3> rodrigues(
    const FixedArray<TData, 3>& k,
    const TData& theta,
    bool check_angle = true)
{
    if (check_angle) {
        assert(std::abs(theta) < TData{2.1 * M_PI});
    }
    FixedArray<TData, 3, 3> K{cross(k)};
    FixedArray<TData, 3, 3> I = fixed_identity_array<TData, 3>();
    return I + std::sin(theta) * K + (1 - std::cos(theta)) * dot(K, K);
}

template <class TData>
FixedArray<TData, 3, 3> tait_bryan_angles_2_matrix(
    const FixedArray<TData, 3>& angles,
    const FixedArray<size_t, 3>& indices = {0, 1, 2})
{
    assert(all(indices < size_t(3)));
    FixedArray<TData, 3, 3> I = fixed_identity_array<TData, 3>();
    FixedArray<TData, 3, 3> result = rodrigues(I[indices(0)], angles(indices(0)));
    result = dot2d(rodrigues(I[indices(1)], angles(indices(1))), result);
    result = dot2d(rodrigues(I[indices(2)], angles(indices(2))), result);
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
        return FixedArray<float, 3>{
            std::atan2(R(2, 1), R(2, 2)),
            std::atan2(-R(2, 0), sy),
            std::atan2(R(1, 0), R(0, 0))};
    } else {
        return FixedArray<float, 3>{
            std::atan2(-R(1, 2), R(1, 1)),
            std::atan2(-R(2, 0), sy),
            0};
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
    if (len2 < 1e-12) {
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

}
