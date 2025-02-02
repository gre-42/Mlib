#include <Mlib/Cv/Project_Depth_Map.hpp>
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <iostream>
#include <random>

using namespace Mlib;
using namespace Mlib::Cv;

void test_homogeneous_jacobian() {
    TransformationMatrix<float, float, 3> M{ FixedArray<float, 3, 4>{ random_array3<float>(ArrayShape{3, 4}, 5) } };
    FixedArray<float, 3> x{ random_array3<float>(ArrayShape{3}, 2) };
    assert_allclose(
        numerical_differentiation<2>([&](
            const FixedArray<float, 3>& xx) { return M.transform(xx).template row_range<0, 2>() / M.transform(xx)(2); },
            x,
            float(1e-3)).to_array(),
        homogeneous_jacobian_dx(M, x).to_array(),
        float(1e-3));
}

void test_projection_jacobian_dx() {
    FixedArray<float, 3> x{ random_array3<float>(ArrayShape{3}, 2) };
    TransformationMatrix<float, float, 3> ke{ FixedArray<float, 3, 4>{ random_array3<float>(ArrayShape{3, 4}, 5) } };
    TransformationMatrix<float, float, 2> ki{ FixedArray<float, 2, 3>::init(
        0.5f, 0.f, 0.1f,
        0.f, 0.7f, 0.2f) };
    auto mm = TransformationMatrix<float, float, 3>{ ki.project(ke.semi_affine()) };
    assert_allclose(
        numerical_differentiation<2>([&](
            const FixedArray<float, 3>& xx) { return mm.transform(xx).template row_range<0, 2>() / mm.transform(xx)(2); },
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
    auto ki = FixedArray<float, 3, 3>::init(
        0.5f, 0.f, 0.1f,
        0.f, 0.7f, 0.2f,
        0.f, 0.f, 1.f);
    assert_allclose(
        numerical_differentiation<2>([&](const FixedArray<float, 6>& kkep){
            return projected_points_1p_1ke(
                x,
                TransformationMatrix<float, float, 2>{ ki },
                k_external(kkep)).template row_range<0, 2>();
        },
        kep,
        float(1e-3)).to_array(),
        Array<float>{
            {-0.0968575f, 0.338674f, -0.0870824f, 0.516891f, 0.f, -0.733554f},
            {-0.155747f, 0.240684f, 0.320494f, 0.f, 0.7236f, -0.536442f}});
    assert_allclose(
        projected_points_jacobian_dke_1p_1ke(x, TransformationMatrix<float, float, 2>{ki}, kep).to_array(),
        Array<float>{
            {-0.096922f, 0.338622f, -0.0870149f, 0.516837f, 0.f, -0.734359f},
            {-0.155774f, 0.240563f, 0.320573f, 0.f, 0.723572f, -0.537037f}},
        float(1e-2));
}

void test_projection_jacobian_ki() {
    TransformationMatrix<float, float, 3> ke = k_external(FixedArray<float, 6>{uniform_random_array<float>(ArrayShape{ 6 }, 1)});
    FixedArray<float, 3> t{ uniform_random_array<float>(ArrayShape{3}, 2) };
    FixedArray<float, 3> x{ uniform_random_array<float>(ArrayShape{3}, 3) };
    FixedArray<float, 4> kip{ uniform_random_array<float>(ArrayShape{4}, 4) };
    assert_allclose(
        numerical_differentiation<2>([&](const FixedArray<float, 4>& kkip){
            return projected_points_1p_1ke(
                x,
                k_internal(kkip),
                ke).template row_range<0, 2>();
        },
        kip,
        float(1e-2)).to_array(),
        Array<float>{
            {53.4374f, 1.f, 0.f, 0.f},
            {0.f, 0.f, 12.5028f, 1.00002f}},
        float(1e-3));
    assert_allclose(
        projected_points_jacobian_dki_1p_1ke(x, ke).to_array(),
        Array<float>{
            {53.4375f, 1.f, 0.f, 0.f},
            {0.f, 0.f, 12.5027f, 1.00002f}},
        float(1e-3));
}

void test_project_depth_map() {
    TransformationMatrix<float, float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>{ Array<float>::load_txt_2d("Data/camera_intrinsic-256x455.m")} };
    Array<float> depth_picture0 = Array<float>::load_binary("Data/Rigid_Motion/depth-0-590.array");
    Array<float> rgb_picture0 = StbImage3::load_from_file("Data/Rigid_Motion/vid001-256x455x24.png").to_float_rgb();

    // Identity
    {
        {
            Array<float> depth_picture1;
            Array<float> rgb_picture1;

            project_depth_map(
                rgb_picture0,
                depth_picture0,
                intrinsic_matrix,
                TransformationMatrix<float, float, 3>::identity(),
                rgb_picture1,
                depth_picture1,
                intrinsic_matrix,
                integral_cast<int>(depth_picture0.shape(1)),
                integral_cast<int>(depth_picture0.shape(0)),
                0.1f,
                1000.f);
            
            draw_nan_masked_rgb(rgb_picture1, 0.f, 1.f).save_to_file("TestOut/projected_depth_map_identity.png");
            draw_nan_masked_grayscale(depth_picture1, 0.f, 0.f).save_to_file("TestOut/projected_depth_map_identity_depth.png");
        }

        {
            Array<float> rgb_picture1_cpu = project_depth_map_cpu(
                rgb_picture0,
                depth_picture0,
                intrinsic_matrix,
                TransformationMatrix<float, float, 3>::identity());
            
            draw_nan_masked_rgb(rgb_picture1_cpu, 0.f, 1.f).save_to_file("TestOut/projected_depth_map_identity_cpu.png");
        }
    }

    // Shift
    {
        TransformationMatrix<float, float, 3> ke = TransformationMatrix<float, float, 3>::identity();
        // ke.t(0) = 0.1f;
        ke.t = FixedArray<float, 3>{ 0.1f, 0.2f, 0.15f };
        // ke.t(2) = 0.1f;
        ke.R = rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, 0.2f);
        lerr() << "ke\n" << ke.semi_affine();

        {
            Array<float> depth_picture1;
            Array<float> rgb_picture1;

            project_depth_map(
                rgb_picture0,
                depth_picture0,
                intrinsic_matrix,
                ke,
                rgb_picture1,
                depth_picture1,
                intrinsic_matrix,
                integral_cast<int>(depth_picture0.shape(1)),
                integral_cast<int>(depth_picture0.shape(0)),
                0.1f,
                1000.f);
            
            draw_nan_masked_rgb(rgb_picture1, 0.f, 1.f).save_to_file("TestOut/projected_depth_map_x.png");
            draw_nan_masked_grayscale(depth_picture1, 0.f, 0.f).save_to_file("TestOut/projected_depth_map_depth_x.png");
        }

        {
            Array<float> rgb_picture1_cpu = project_depth_map_cpu(
                rgb_picture0,
                depth_picture0,
                intrinsic_matrix,
                ke);
            
            draw_nan_masked_rgb(rgb_picture1_cpu, 0.f, 1.f).save_to_file("TestOut/projected_depth_map_cpu_x.png");
        }
    }
}

int main(int argc, char** argv) {
    try {
        test_homogeneous_jacobian();
        test_tait_bryan_angles_jacobian();
        test_projection_jacobian_dx();
        test_projection_jacobian_ke();
        test_projection_jacobian_ki();
        test_project_depth_map();
    } catch (const std::runtime_error& e) {
        lerr() << "ERROR: " << e.what();
        return 1;
    }
    return 0;
}
