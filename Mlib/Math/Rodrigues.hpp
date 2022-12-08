#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Geometry/Cross.hpp>

namespace Mlib{

template <class TData>
Array<TData> rodrigues2(const Array<TData>& k, const TData& theta);

template <class TData>
Array<TData> tait_bryan_angles_2_matrix(const Array<TData>& angles);

template <class TData>
Array<TData> rodrigues1(const Array<TData>& k) {
    assert(all(k.shape() == ArrayShape{3}));
    if (sum(squared(k)) < 1e-12) {
        static const Array<TData> I = identity_array<TData>(3);
        return I;
    } else {
        TData len_k = std::sqrt(sum(norm(k)));
        return rodrigues2(k / len_k, len_k);
    }
}

template <class TData>
Array<TData> rodrigues2(const Array<TData>& k, const TData& theta) {
    assert(all(k.shape() == ArrayShape{3}));

    // slow
    // Array<TData> K{cross(k)};
    // static const Array<TData> I = identity_array<TData>(3);
    // return I + std::sin(theta) * K + (1 - std::cos(theta)) * dot(K, K);

    static Array<TData> K = zeros<TData>(ArrayShape{3, 3});
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
    // Array<TData> result = rodrigues2(I[0], alpha);
    // result = (result, rodrigues2(I[1], beta).T());
    // result = (result, rodrigues2(I[2], gamma).T());
    // return result;

    assert(angles.length() == 3);
    static const Array<TData> I = identity_array<TData>(3);
    static const Array<TData> I0 = I[0];
    static const Array<TData> I1 = I[1];
    static const Array<TData> I2 = I[2];
    static Array<TData> tmp{ArrayShape{3, 3}};
    Array<TData> ro = rodrigues2(I0, angles(0));
    dot2d(rodrigues2(I1, angles(1)), ro, tmp);
    dot2d(rodrigues2(I2, angles(2)), tmp, ro);
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

}
