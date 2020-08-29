#include "Homogeneous.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>

using namespace Mlib;

Array<float> Mlib::intrinsic_times_inverse(
    const Array<float>& intrinsic_matrix,
    const Array<float>& inv_rhs)
{
    Array<float> ike = inverted_homogeneous_3x4(inv_rhs);
    return dot(intrinsic_matrix, ike);
}

Array<float> Mlib::reconstruction_times_inverse(
    const Array<float>& recon_lhs,
    const Array<float>& inv_rhs)
{
    return lstsq_chol(homogenized_4x4(inv_rhs).T(), homogenized_4x4(recon_lhs).T()).T();
}

/**
 * Find a reconstruction-matrix E converting coefficients b in B
 * to a, x -> a, where a is a point
 * in reference A.
 * Note that B (and A) are projection-matrices and therefore B^{-1} * b is
 * a point in global coordinates, projected, by A,
 * to coordinates in reference A^{-1}.
 * A * x = a
 * A * B^{-1} * b = a
 * => E = (B^{-T} A^T)^T
 * Transposition because lstsq_chol inverts the left argument
 * of the multiplication.
 */
Array<float> Mlib::inverse_projection_in_reference(
    const Array<float>& reference,
    const Array<float>& b)
{
    auto& a = reference;
    return lstsq_chol(homogenized_4x4(b).T(), homogenized_4x4(a).T()).T();
}

/**
 * Find a projection-matrix E converting coefficients a in A^{-1}
 * to b, a -> b, where a is a coefficient in reference A^{-1}.
 * Note that B (and A) are projection-matrices and therefore A^{-1} * a is
 * a point in global coordinates, projected, by B, to coefficients
 * in reference B^{-1}.
 * B * x = b
 * B * A^{-1} a = b
 * => E = (A^{-T} B^T)^T
 * Transposition because lstsq_chol inverts the left argument
 * of the multiplication.
 */
Array<float> Mlib::projection_in_reference(
    const Array<float>& reference,
    const Array<float>& b)
{
    auto& a = reference;
    return lstsq_chol(homogenized_4x4(a).T(), homogenized_4x4(b).T()).T().row_range(0, 3);
}

/**
 * Derivation 1:
 * Find a reconstruction-matrix E converting coefficients c in B
 * to \tilde{x}, x -> \tilde{x} with reference A.
 * A^{-1} * x = \tilde{x}.
 * A^{-1} * B * c = \tilde{x}
 *
 * Derivation 2:
 * Find a reconstruction-matrix E s.t. A
 * is the reference coordinate-system.
 * A^{-1} * A * x = I * x
 * A^{-1} * B * y = (A^{-1} * B) * y
 * => E = A^{-1} * B
 */
Array<float> Mlib::reconstruction_in_reference(
    const Array<float>& reference,
    const Array<float>& b)
{
    auto& a = reference;
    return lstsq_chol(homogenized_4x4(a), homogenized_4x4(b));
}

void Mlib::invert_t_R(
    const Array<float>& t,
    const Array<float>& R,
    Array<float>& ti,
    Array<float>& Ri)
{
    assert(all(R.shape() == ArrayShape{3, 3}));
    assert(all(t.shape() == ArrayShape{3}));
    Ri = R.T();
    ti = -dot1d(Ri, t);
}

void Mlib::invert_t_R(
    const FixedArray<float, 3>& t,
    const FixedArray<float, 3, 3>& R,
    FixedArray<float, 3>& ti,
    FixedArray<float, 3, 3>& Ri)
{
    Ri = R.T();
    ti = -dot1d(Ri, t);
}

Array<float> Mlib::t3_from_Nx4(const Array<float>& a, size_t nrows) {
    assert(nrows == 3 || nrows == 4);
    assert(all(a.shape() == ArrayShape{nrows, 4}));
    return Array<float>{
        a(0, 3),
        a(1, 3),
        a(2, 3)};
}

Array<float> Mlib::R3_from_Nx4(const Array<float>& a, size_t nrows) {
    assert(nrows == 3 || nrows == 4);
    assert(all(a.shape() == ArrayShape{nrows, 4}));
    return Array<float>{
            {a(0, 0), a(0, 1), a(0, 2)},
            {a(1, 0), a(1, 1), a(1, 2)},
            {a(2, 0), a(2, 1), a(2, 2)}};
}

Array<float> Mlib::inverted_homogeneous_3x4(const Array<float>& ke) {
    return lstsq_chol(
        homogenized_4x4(ke),
        identity_array<float>(4)).row_range(0, 3);
}

// Function is inlined
// FixedArray<float, 3> Mlib::t3_from_4x4(const FixedArray<float, 4, 4>& a) {
//     return FixedArray<float, 3>{
//         a(0, 3),
//         a(1, 3),
//         a(2, 3)};
// }

FixedArray<float, 3, 3> Mlib::R3_from_4x4(const FixedArray<float, 4, 4>& a) {
    return FixedArray<float, 3, 3>{
        a(0, 0), a(0, 1), a(0, 2),
        a(1, 0), a(1, 1), a(1, 2),
        a(2, 0), a(2, 1), a(2, 2)};
}

FixedArray<float, 4, 4> Mlib::inverted_scaled_se3(const FixedArray<float, 4, 4>& m) {
    auto R = R3_from_4x4(m);
    auto scale2 = sum(squared(R)) / 3;
    return assemble_inverse_homogeneous_4x4(R / scale2, t3_from_4x4(m));
}

/**
 * See also: Mlib::assemble_inverse_homogeneous_3x4, Mlib::invert_t_R,
 * the alternative implementation in "reconstructed_point"
 */
void Mlib::homogeneous_to_inverse_t_R(
    const Array<float>& ke,
    Array<float>& t,
    Array<float>& R)
{
    assert(all(ke.shape() == ArrayShape{3, 4}));

    // y = ki * ke * x
    // p = camera-position
    // [0; 0; 0; 1] = ke * p

    Array<float> ike = inverted_homogeneous_3x4(ke);
    t = t3_from_Nx4(ike, 3);
    R = R3_from_Nx4(ike, 3);
    // assert_allclose(R3_from_Nx4(ke, 3).T(), R);
}

Array<float> Mlib::assemble_homogeneous_3x4(const Array<float>& R, const Array<float>& t) {
    assert(all(R.shape() == ArrayShape{3, 3}));
    assert(all(t.shape() == ArrayShape{3}));
    return Array<float>{
            {R(0, 0), R(0, 1), R(0, 2), t(0)},
            {R(1, 0), R(1, 1), R(1, 2), t(1)},
            {R(2, 0), R(2, 1), R(2, 2), t(2)}};
}

FixedArray<float, 4, 4> Mlib::assemble_homogeneous_4x4(const FixedArray<float, 3, 3>& R, const FixedArray<float, 3>& t) {
    return FixedArray<float, 4, 4>{
            R(0, 0), R(0, 1), R(0, 2), t(0),
            R(1, 0), R(1, 1), R(1, 2), t(1),
            R(2, 0), R(2, 1), R(2, 2), t(2),
            0, 0, 0, 1};
}

Array<float> Mlib::assemble_inverse_homogeneous_3x4(const Array<float>& R, const Array<float>& t) {
    // slow
    // return lstsq_chol(
    //     homogenized_4x4(assemble_homogeneous_3x4(R, t)),
    //     identity_array<float>(4)).row_range(0, 3);

    // fast
    Array<float> Ri;
    Array<float> ti;
    invert_t_R(t, R, ti, Ri);
    return assemble_homogeneous_3x4(Ri, ti);
}

FixedArray<float, 4, 4> Mlib::assemble_inverse_homogeneous_4x4(const FixedArray<float, 3, 3>& R, const FixedArray<float, 3>& t) {
    FixedArray<float, 3, 3> Ri;
    FixedArray<float, 3> ti;
    invert_t_R(t, R, ti, Ri);
    return assemble_homogeneous_4x4(Ri, ti);
}

Array<float> Mlib::homogenized_4x4(const Array<float>& a) {
    assert(all(a.shape() == ArrayShape{3, 4}));
    Array<float> hke = identity_array<float>(4);
    hke.reshaped(ArrayShape{3, 4}) = a;
    return hke;
}

Array<float> Mlib::homogenized_4(const Array<float>& a) {
    assert(all(a.shape() == ArrayShape{3}));
    return Array<float>{a(0), a(1), a(2), 1};
}

// Function is inlined
// FixedArray<float, 4> Mlib::homogenized_4(const FixedArray<float, 3>& a) {
//     return FixedArray<float, 4>{a(0), a(1), a(2), 1};
// }

Array<float> Mlib::homogenized_3(const Array<float>& a) {
    assert(all(a.shape() == ArrayShape{2}));
    return Array<float>{a(0), a(1), 1};
}

FixedArray<float, 3> Mlib::homogenized_3(const FixedArray<float, 2>& a) {
    return FixedArray<float, 3>{a(0), a(1), 1};
}

Array<float> Mlib::homogenized_Nx3(const Array<float>& a) {
    assert(a.ndim() == 2);
    assert(a.shape(1) == 2);
    Array<float> result{ArrayShape{a.shape(0), 3}};
    for(size_t r = 0; r < a.shape(0); ++r) {
        result(r, 0) = a(r, 0);
        result(r, 1) = a(r, 1);
        result(r, 2) = 1;
    }
    return result;
}

Array<float> Mlib::homogenized_Nx4(const Array<float>& a, float value) {
    assert(a.ndim() == 2);
    assert(a.shape(1) == 3);
    Array<float> result{ArrayShape{a.shape(0), 4}};
    for(size_t r = 0; r < a.shape(0); ++r) {
        result(r, 0) = a(r, 0);
        result(r, 1) = a(r, 1);
        result(r, 2) = a(r, 2);
        result(r, 3) = value;
    }
    return result;
}

Array<float> Mlib::dehomogenized_Nx3(const Array<float>& a, float value) {
    assert(a.ndim() == 2);
    assert(a.shape(1) == 4);
    Array<float> result{ArrayShape{a.shape(0), 3}};
    for(size_t r = 0; r < a.shape(0); ++r) {
        result(r, 0) = a(r, 0);
        result(r, 1) = a(r, 1);
        result(r, 2) = a(r, 2);
        assert(std::abs(a(r, 3) - value) < 1e-12);
    }
    return result;
}

Array<float> Mlib::dehomogenized_Nx2(const Array<float>& a, float value) {
    assert(a.ndim() == 2);
    assert(a.shape(1) == 3);
    Array<float> result{ArrayShape{a.shape(0), 2}};
    for(size_t r = 0; r < a.shape(0); ++r) {
        result(r, 0) = a(r, 0);
        result(r, 1) = a(r, 1);
        assert(std::abs(a(r, 2) - value) < 1e-12);
    }
    return result;
}

Array<float> Mlib::dehomogenized_2(const Array<float>& a, float value) {
    assert(all(a.shape() == ArrayShape{3}));
    assert(std::abs(a(2) - value) < 1e-12);
    return Array<float>{a(0), a(1)};
}

FixedArray<float, 2> Mlib::dehomogenized_2(const FixedArray<float, 3>& a, float value) {
    assert(std::abs(a(2) - value) < 1e-12);
    return FixedArray<float, 2>{a(0), a(1)};
}

Array<float> Mlib::dehomogenized_3(const Array<float>& a) {
    assert(all(a.shape() == ArrayShape{4}));
    assert(std::abs(a(3) - 1) < 1e-12);
    return Array<float>{a(0), a(1), a(2)};
}

// Function is inlined
// FixedArray<float, 3> Mlib::dehomogenized_3(const FixedArray<float, 4>& a) {
//     assert(std::abs(a(3) - 1) < 1e-12);
//     return FixedArray<float, 3>{a(0), a(1), a(2)};
// }

Array<float> Mlib::dehomogenized_3x4(const Array<float>& a) {
    assert(all(a.shape() == ArrayShape{4, 4}));
    return a.row_range(0, 3);
}

Array<float> Mlib::homogeneous_jacobian_dx(const Array<float>& M, const Array<float>& x) {
    assert(M.ndim() == 2);
    assert(M.shape(1) > 0);
    assert(all(x.shape() == ArrayShape{M.shape(1)}));
    const auto m = M.row_range(0, M.shape(0) - 1);
    const auto Mx = dot1d(M, x);
    const auto mx = Mx.row_range(0, Mx.length() - 1);
    const float bx = Mx(Mx.length() - 1);

    const auto mx_2d = mx.reshaped(ArrayShape{mx.length(), 1});
    const auto b_2d = M.row_range(M.shape(0) - 1, M.shape(0));

    // M = [m0; m1 ... ; b]
    // d/dx m'x/(b'x) = (m(0)(b'x) - (m'x)*b(0)) / squared(b'x)

    return ((m * bx) - dot(mx_2d, b_2d)) / squared(bx);
}
