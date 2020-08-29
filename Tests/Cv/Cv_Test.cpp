#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <iostream>
#include <random>

using namespace Mlib;
using namespace Mlib::Cv;

void test_tait_bryan_angles_2_matrix() {
    {
        Array<float> r = tait_bryan_angles_2_matrix(Array<float>{0.f, 0.f, 0.f});
        assert_allclose(r, identity_array<float>(3));
        assert_allclose(outer(r, r), identity_array<float>(3));
    }

    {
        Array<float> r = tait_bryan_angles_2_matrix(Array<float>{1.f, 0.f, 0.f});
        assert_allclose(outer(r, r), identity_array<float>(3));
        assert_allclose(dot(r.T(), r), identity_array<float>(3));
    }

    {
        Array<float> r = tait_bryan_angles_2_matrix(Array<float>{0.1f, 0.2f, 0.3f});
        assert_allclose(outer(r, r), identity_array<float>(3));
        assert_allclose(dot(r.T(), r), identity_array<float>(3));
    }
}

void test_homogeneous_jacobian() {
    Array<float> M = random_array3<float>(ArrayShape{3, 4}, 5);
    Array<float> x = random_array3<float>(ArrayShape{4}, 2);
    assert_allclose(
        numerical_differentiation([&](
            const Array<float>& xx){ return dot1d(M.row_range(0, 2), xx) / dot(M[2], xx)(); },
            x,
            float(1e-3)),
        homogeneous_jacobian_dx(M, x),
        1e-3);
}

void test_tait_bryan_angles_jacobian() {
    {
        // Test gradient with a single angle.
        float theta = 0.3;
        Array<float> x = random_array4<float>(ArrayShape{3}, 3);
        assert_allclose(
            numerical_differentiation([&](
                const Array<float>& ttheta){ return dot1d(tait_bryan_angles_2_matrix(Array<float>{ttheta(0), 0.f, 0.f}), x); },
                Array<float>{theta},
                float(1e-4)).flattened(),
            rodrigues_gradient_dtheta(Array<float>{1, 0, 0}, theta, x),
            float(1e-3));
    }

    {
        // Test jacobian with 3 angles.
        Array<float> theta = random_array4<float>(ArrayShape{3}, 1);
        Array<float> x = random_array4<float>(ArrayShape{3}, 2);

        assert_allclose(
            numerical_differentiation([&](
                const Array<float>& ttheta){ return dot1d(tait_bryan_angles_2_matrix(ttheta), x); },
                theta,
                float(1e-4)),
            tait_bryan_angles_dtheta(theta, x),
            float(1e-3));
    }
}

void test_projection_jacobian_ke() {
    Array<float> kep = random_array3<float>(ArrayShape{6}, 2);
    Array<float> t = random_array3<float>(ArrayShape{3}, 2);
    Array<float> x = random_array3<float>(ArrayShape{3}, 2);
    Array<float> ki{
        {0.5, 0, 0.1},
        {0, 0.7, 0.2},
        {0, 0, 1}};
    assert_allclose(
        numerical_differentiation([&](const Array<float>& kkep){
            return projected_points_1p_1ke(
                homogenized_4(x),
                ki,
                k_external(kkep)).row_range(0, 2);
        },
        kep,
        float(1e-2)),
        Array<float>{
            {0.0350952, 0.301784, -0.15803, 0.536746, 0, -0.515664},
            {-0.383005, 0.220874, -0.0106782, 0, 0.751445, -0.28733}});
    assert_allclose(
        projected_points_jacobian_dke_1p_1ke(homogenized_4(x), ki, kep),
        Array<float>{
            {0.0338736, 0.300373, -0.158062, 0.536745, 0, -0.521198},
            {-0.383656, 0.220084, -0.00957519, 0, 0.751443, -0.290414}},
        1e-2);
}

void test_projection_jacobian_ki() {
    Array<float> ke = k_external(random_array4<float>(ArrayShape{6}, 1));
    Array<float> t = random_array4<float>(ArrayShape{3}, 2);
    Array<float> x = random_array4<float>(ArrayShape{3}, 3);
    Array<float> kip = random_array4<float>(ArrayShape{4}, 4);
    assert_allclose(
        numerical_differentiation([&](const Array<float>& kkip){
            return projected_points_1p_1ke(
                homogenized_4(x),
                k_internal(kkip),
                ke).row_range(0, 2);
        },
        kip,
        float(1e-2)),
        Array<float>{
            {12.9208, 0.999975, 0, 0},
            {0, 0, 9.05638, 1.00002}},
        1e-4);
    assert_allclose(
        projected_points_jacobian_dki_1p_1ke(homogenized_4(x), kip, ke),
        Array<float>{
            {12.9208, 0.999975, 0, 0},
            {0, 0, 9.05638, 1.00002}},
        1e-4);
}

void test_inverse_tait_bryan_angles() {
    Array<float> kep = random_array4<float>(ArrayShape{3}, 1);
    assert_allclose(kep, matrix_2_tait_bryan_angles(tait_bryan_angles_2_matrix(kep)));
    assert_allclose(
        Array<float>{kep(0), kep(1), 0},
        matrix_2_tait_bryan_angles(
            tait_bryan_angles_2_matrix(Array<float>{kep(0), kep(1), 0}),
            true));  // true == force_singular
}

void test_rodrigues_fixed() {
    Array<float> k = random_array4<float>(ArrayShape{3}, 1);
    auto kf = FixedArray<float, 3>{k};
    FixedArray<float, 3, 3> rf = rodrigues(kf);
    Array<float> r = rodrigues(k);
    assert_allclose(r, rf.to_array());
}

void test_fixed_tait_bryan_angles_2_matrix() {
    Array<float> k = random_array4<float>(ArrayShape{3}, 1);
    auto kf = FixedArray<float, 3>{k};
    auto rf = tait_bryan_angles_2_matrix(kf);
    auto r = tait_bryan_angles_2_matrix(k);
    assert_allclose(r, rf.to_array());

    assert_allclose(k, matrix_2_tait_bryan_angles(rf).to_array());
    kf(2) = 0;
    rf = tait_bryan_angles_2_matrix(kf);
    assert_allclose(kf.to_array(), matrix_2_tait_bryan_angles(rf, true).to_array());  // true=force_singular
}

int main(int argc, char** argv) {
    test_tait_bryan_angles_2_matrix();
    test_homogeneous_jacobian();
    test_tait_bryan_angles_jacobian();
    test_projection_jacobian_ke();
    test_projection_jacobian_ki();
    test_inverse_tait_bryan_angles();
    test_rodrigues_fixed();
    test_fixed_tait_bryan_angles_2_matrix();
    return 0;
}
