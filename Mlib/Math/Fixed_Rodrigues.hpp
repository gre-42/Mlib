#pragma once
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib{

template <class TData>
FixedArray<TData, 3, 3> rodrigues(const FixedArray<TData, 3>& k) {
    if (sum(squared(k)) < 1e-12) {
        return fixed_identity_array<TData, 3>();
    } else {
        TData len_k = std::sqrt(sum(norm(k)));
        return rodrigues(k / len_k, len_k);
    }
}

template <class TData>
FixedArray<TData, 3, 3> rodrigues(const FixedArray<TData, 3>& k, const TData& theta) {
    FixedArray<TData, 3, 3> K{cross(k)};
    FixedArray<TData, 3, 3> I = fixed_identity_array<TData, 3>();
    return I + std::sin(theta) * K + (1 - std::cos(theta)) * dot(K, K);
}

template <class TData>
FixedArray<TData, 3, 3> tait_bryan_angles_2_matrix(const FixedArray<TData, 3>& angles)
{
    FixedArray<TData, 3, 3> I = fixed_identity_array<TData, 3>();
    FixedArray<TData, 3, 3> result = rodrigues(I[0], angles(0));
    result = dot2d(rodrigues(I[1], angles(1)), result);
    result = dot2d(rodrigues(I[2], angles(2)), result);
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

}
