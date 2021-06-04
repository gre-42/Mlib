#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
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
    TransformationMatrix<float, 3> M{ FixedArray<float, 3, 4>{ random_array3<float>(ArrayShape{3, 4}, 5) } };
    FixedArray<float, 3> x{ random_array3<float>(ArrayShape{3}, 2) };
    assert_allclose(
        numerical_differentiation<2>([&](
            const FixedArray<float, 3>& xx) { return M.transform(xx).row_range<0, 2>() / M.transform(xx)(2); },
            x,
            float(1e-3)).to_array(),
        homogeneous_jacobian_dx(M, x).to_array(),
        float(1e-3));
}

void test_projection_jacobian_dx() {
    FixedArray<float, 3> x{ random_array3<float>(ArrayShape{4}, 2) };
    TransformationMatrix<float, 3> ke{ FixedArray<float, 3, 4>{ random_array3<float>(ArrayShape{3, 4}, 5) } };
    TransformationMatrix<float, 2> ki{ FixedArray<float, 2, 3>{
        0.5f, 0.f, 0.1f,
        0.f, 0.7f, 0.2f } };
    auto mm = TransformationMatrix<float, 3>{ ki.project(ke.semi_affine()) };
    assert_allclose(
        numerical_differentiation<2>([&](
            const FixedArray<float, 3>& xx) { return mm.transform(xx).row_range<0, 2>() / mm.transform(xx)(2); },
            x,
            float(1e-3)).to_array(),
        projected_points_jacobian_dx_1p_1ke(x, ki, ke).to_array(),
        float(1e-3));
}

void test_tait_bryan_angles_jacobian() {
    {
        // Test gradient with a single angle.
        float theta = 0.3f;
        FixedArray<float, 3> x{ uniform_random_array<float>(ArrayShape{3}, 3) };
        assert_allclose(
            numerical_differentiation<3>([&](
                const FixedArray<float, 1>& ttheta){ return dot1d(tait_bryan_angles_2_matrix(FixedArray<float, 3>{ttheta(0), 0.f, 0.f}), x); },
                FixedArray<float, 1>{theta},
                float(1e-4)).to_array().flattened(),
            rodrigues_gradient_dtheta(FixedArray<float, 3>{1.f, 0.f, 0.f}, theta, x).to_array(),
            float(1e-3));
    }

    {
        // Test jacobian with 3 angles.
        FixedArray<float, 3> theta{ uniform_random_array<float>(ArrayShape{3}, 1) };
        FixedArray<float, 3> x{ uniform_random_array<float>(ArrayShape{3}, 2) };

        assert_allclose(
            numerical_differentiation<3>([&](
                const FixedArray<float, 3>& ttheta){ return dot1d(tait_bryan_angles_2_matrix(ttheta), x); },
                theta,
                float(1e-4)).to_array(),
            tait_bryan_angles_dtheta(theta, x).to_array(),
            float(1e-3));
    }
}

void test_projection_jacobian_ke() {
    FixedArray<float, 6> kep{ random_array3<float>(ArrayShape{6}, 2) };
    FixedArray<float, 3> t{ random_array3<float>(ArrayShape{3}, 2) };
    FixedArray<float, 3> x{ random_array3<float>(ArrayShape{3}, 2) };
    FixedArray<float, 3, 3> ki{
        0.5f, 0.f, 0.1f,
        0.f, 0.7f, 0.2f,
        0.f, 0.f, 1.f };
    assert_allclose(
        numerical_differentiation<2>([&](const FixedArray<float, 6>& kkep){
            return projected_points_1p_1ke(
                x,
                TransformationMatrix<float, 2>{ ki },
                k_external(kkep)).row_range<0, 2>();
        },
        kep,
        float(1e-2)).to_array(),
        Array<float>{
            {0.0350952f, 0.301784f, -0.15803f, 0.536746f, 0.f, -0.515664f},
            {-0.383005f, 0.220874f, -0.0106782f, 0.f, 0.751445f, -0.28733f}});
    assert_allclose(
        projected_points_jacobian_dke_1p_1ke(x, TransformationMatrix<float, 2>{ki}, kep).to_array(),
        Array<float>{
            {0.0338736f, 0.300373f, -0.158062f, 0.536745f, 0.f, -0.521198f},
            {-0.383656f, 0.220084f, -0.00957519f, 0, 0.751443f, -0.290414f}},
        float{ 1e-2 });
}

void test_projection_jacobian_ki() {
    TransformationMatrix<float, 3> ke = k_external(FixedArray<float, 6>{uniform_random_array<float>(ArrayShape{ 6 }, 1)});
    FixedArray<float, 3> t{ uniform_random_array<float>(ArrayShape{3}, 2) };
    FixedArray<float, 3> x{ uniform_random_array<float>(ArrayShape{3}, 3) };
    FixedArray<float, 4> kip{ uniform_random_array<float>(ArrayShape{4}, 4) };
    assert_allclose(
        numerical_differentiation<2>([&](const FixedArray<float, 4>& kkip){
            return projected_points_1p_1ke(
                x,
                k_internal(kkip),
                ke).row_range<0, 2>();
        },
        kip,
        float(1e-2)).to_array(),
        Array<float>{
            {12.9208f, 0.999975f, 0.f, 0.f},
            {0.f, 0.f, 9.05638f, 1.00002f}},
        float{ 1e-4 });
    assert_allclose(
        projected_points_jacobian_dki_1p_1ke(x, ke).to_array(),
        Array<float>{
            {12.9208f, 0.999975f, 0.f, 0.f},
            {0.f, 0, 9.05638f, 1.00002f}},
        float{ 1e-4 });
}

void test_inverse_tait_bryan_angles() {
    Array<float> kep = uniform_random_array<float>(ArrayShape{3}, 1);
    assert_allclose(kep, matrix_2_tait_bryan_angles(tait_bryan_angles_2_matrix(kep)));
    assert_allclose(
        Array<float>{kep(0), kep(1), 0},
        matrix_2_tait_bryan_angles(
            tait_bryan_angles_2_matrix(Array<float>{kep(0), kep(1), 0}),
            true));  // true == force_singular
}

void test_rodrigues_fixed() {
    Array<float> k = uniform_random_array<float>(ArrayShape{3}, 1);
    auto kf = FixedArray<float, 3>{k};
    FixedArray<float, 3, 3> rf = rodrigues(kf);
    Array<float> r = rodrigues(k);
    assert_allclose(r, rf.to_array());
}

void test_fixed_tait_bryan_angles_2_matrix() {
    Array<float> k = uniform_random_array<float>(ArrayShape{3}, 1);
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
    try {
        test_tait_bryan_angles_2_matrix();
        test_homogeneous_jacobian();
        test_tait_bryan_angles_jacobian();
        test_projection_jacobian_dx();
        test_projection_jacobian_ke();
        test_projection_jacobian_ki();
        test_inverse_tait_bryan_angles();
        test_rodrigues_fixed();
        test_fixed_tait_bryan_angles_2_matrix();
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
