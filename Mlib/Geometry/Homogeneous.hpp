#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

Array<float> intrinsic_times_inverse(
    const Array<float>& intrinsic_matrix,
    const Array<float>& inv_rhs);

Array<float> reconstruction_times_inverse(
    const Array<float>& recon_lhs,
    const Array<float>& inv_rhs);

Array<float> inverse_projection_in_reference(
    const Array<float>& reference,
    const Array<float>& b);

Array<float> projection_in_reference(
    const Array<float>& reference,
    const Array<float>& b);

Array<float> reconstruction_in_reference(
    const Array<float>& reference,
    const Array<float>& b);

void invert_t_R(
    const Array<float>& t,
    const Array<float>& R,
    Array<float>& ti,
    Array<float>& Ri);

void invert_t_R(
    const FixedArray<float, 3>& t,
    const FixedArray<float, 3, 3>& R,
    FixedArray<float, 3>& ti,
    FixedArray<float, 3, 3>& Ri);

Array<float> t3_from_Nx4(const Array<float>& a, size_t nrows);

Array<float> R3_from_Nx4(const Array<float>& a, size_t nrows);

FixedArray<float, 4, 4> inverted_scaled_se3(const FixedArray<float, 4, 4>& m);

inline FixedArray<float, 3> z3_from_4x4(const FixedArray<float, 4, 4>& a) {
    return FixedArray<float, 3>{
        a(0, 2),
        a(1, 2),
        a(2, 2)};
}

inline FixedArray<float, 3> z3_from_3x3(const FixedArray<float, 3, 3>& a) {
    return FixedArray<float, 3>{
        a(0, 2),
        a(1, 2),
        a(2, 2)};
}

inline FixedArray<float, 3> t3_from_4x4(const FixedArray<float, 4, 4>& a) {
    return FixedArray<float, 3>{
        a(0, 3),
        a(1, 3),
        a(2, 3)};
}

FixedArray<float, 3, 3> R3_from_4x4(const FixedArray<float, 4, 4>& a);

Array<float> inverted_homogeneous_3x4(const Array<float>& ke);

void homogeneous_to_inverse_t_R(const Array<float>& ke, Array<float>& t, Array<float>& R);

Array<float> assemble_homogeneous_3x4(const Array<float>& R, const Array<float>& t);

FixedArray<float, 4, 4> assemble_homogeneous_4x4(const FixedArray<float, 3, 3>& R, const FixedArray<float, 3>& t);

Array<float> assemble_inverse_homogeneous_3x4(const Array<float>& R, const Array<float>& t);

FixedArray<float, 4, 4> assemble_inverse_homogeneous_4x4(const FixedArray<float, 3, 3>& R, const FixedArray<float, 3>& t);

Array<float> homogenized_4x4(const Array<float>& a);

Array<float> homogenized_4(const Array<float>& a);

inline FixedArray<float, 4> homogenized_4(const FixedArray<float, 3>& a) {
    return FixedArray<float, 4>{a(0), a(1), a(2), 1};
}

Array<float> homogenized_3(const Array<float>& a);

FixedArray<float, 3> homogenized_3(const FixedArray<float, 2>& a);

Array<float> homogenized_Nx3(const Array<float>& a);

Array<float> homogenized_Nx4(const Array<float>& a, float value = 1);

Array<float> dehomogenized_Nx3(const Array<float>& a, float value = 1);

Array<float> dehomogenized_Nx2(const Array<float>& a, float value = 1);

Array<float> dehomogenized_2(const Array<float>& a, float value = 1);

FixedArray<float, 2> dehomogenized_2(const FixedArray<float, 3>& a, float value = 1);

Array<float> dehomogenized_3(const Array<float>& a);

inline FixedArray<float, 3> dehomogenized_3(const FixedArray<float, 4>& a) {
    assert(std::abs(a(3) - 1) < 1e-12);
    return FixedArray<float, 3>{a(0), a(1), a(2)};
}

Array<float> dehomogenized_3x4(const Array<float>& a);

Array<float> homogeneous_jacobian_dx(const Array<float>& M, const Array<float>& x);

}
