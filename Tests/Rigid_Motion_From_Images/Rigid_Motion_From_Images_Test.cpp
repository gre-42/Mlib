#include <Mlib/Cv/Rigid_Motion/Rigid_Motion_Roundtrip.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Bgr565Bitmap.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Math/Fixed_Test.hpp>
#include <Mlib/Sfm/Data_Generators/Folder_Data_Generator.hpp>
#include <Mlib/Sfm/Pipelines/Reconstruction/Template_Patch_Pipeline.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images_Robust.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images_Smooth.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <iostream>
#include <random>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Rmfi;

void test_jacobian() {
    TransformationMatrix<float, float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>::init(
        0.5f, 0.f, 0.9f,
        0.f, 0.7f, 0.2f,
        0.f, 0.f, 1.f) };
    FixedArray<float, 6> kep{ uniform_random_array<float>(ArrayShape{6}, 3) };
    FixedArray<float, 2> x{ uniform_random_array<float>(ArrayShape{2}, 2) };
    float depth = 2.345f;
    FixedArray<float, 2, 6> num = numerical_differentiation<2>([&](const FixedArray<float, 6>& kkep){
            return transform_coordinates(intrinsic_matrix, k_external(kkep), x, depth);
        },
        kep);
    FixedArray<float, 2, 6> an = projected_points_jacobian_dke_1p_1ke_lifting(x, depth, intrinsic_matrix, intrinsic_matrix, kep);
    assert_allclose(num, an, float{ 1e-3 });
}

void test_intensity_jacobian() {
    TransformationMatrix<float, float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>::init(
        0.5f, 0.f, 0.9f,
        0.f, 0.7f, 0.2f,
        0.f, 0.f, 1.f) };
    FixedArray<float, 6> kep{0.2f, 0.1f, 0.4f, 0.12f, 0.34f, 0.56f};
    Array<float> im_r = uniform_random_array<float>(ArrayShape{2, 4, 5}, 1);
    Array<float> im_l = uniform_random_array<float>(ArrayShape{2, 4, 5}, 2);
    Array<float> im_r_depth = uniform_random_array<float>(ArrayShape{4, 5}, 1) * 0.2f + 2.f;
    Array<float> im_r_f = multichannel_central_gradient_filter(im_r);
    Array<float> im_l_f = multichannel_central_gradient_filter(im_l);
    Array<float> ij = intensity_jacobian(im_r_f, im_l_f, im_r_depth, intrinsic_matrix, intrinsic_matrix, kep);
    assert(all(ij.shape() == ArrayShape{2, 4, 5, 6}));
    // assert_isclose(sum(squared(ij)), 611.727f, float(1e-2));

    assert_allclose(
        intensity_jacobian(im_r_f, im_l_f, im_r_depth, intrinsic_matrix, intrinsic_matrix, kep),
        intensity_jacobian_fast(im_r_f, im_l_f, im_r_depth, intrinsic_matrix, intrinsic_matrix, kep),
        float{ 1e-4 });
}

void test_rigid_motion_from_images() {
    TransformationMatrix<float, float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>{ Array<float>::load_txt_2d("Data/camera_intrinsic-256x455.m")} };
    Array<float> depth0 = Array<float>::load_binary("Data/Rigid_Motion/depth-0-590.array");
    Array<float> depth1 = Array<float>::load_binary("Data/Rigid_Motion/depth-200-790.array");
    Array<float> im0 = StbImage3::load_from_file("Data/Rigid_Motion/vid001-256x455x24.png").to_float_rgb();
    Array<float> im1 = StbImage3::load_from_file("Data/Rigid_Motion/vid021-256x455x24.png").to_float_rgb();

    draw_nan_masked_rgb(im0, 0, 1).save_to_file("TestOut/rmfi-0.png");
    draw_nan_masked_rgb(im1, 0, 1).save_to_file("TestOut/rmfi-1.png");
    Array<float> im0m = d_pr_bilinear(im0, im1, depth0, intrinsic_matrix, intrinsic_matrix, TransformationMatrix<float, float, 3>::identity());
    draw_nan_masked_rgb(im0m, -0.1f, 0.1f).save_to_file("TestOut/rmfi-d_pr.png");

    {
        TransformationMatrix<float, float, 3> ke_id = rigid_motion_from_images_smooth(im0, im0, depth0, intrinsic_matrix, intrinsic_matrix, {1.f});
        assert_allclose(ke_id.affine(), fixed_identity_array<float, 4>());
    }
    {
        TransformationMatrix<float, float, 3> ke_s = rigid_motion_from_images_smooth(im0, im1, depth0, intrinsic_matrix, intrinsic_matrix, { 3.f });
        Array<float> im1t_s = d_pr_bilinear(im0, im1, depth0, intrinsic_matrix, intrinsic_matrix, ke_s);

        TransformationMatrix<float, float, 3> ke_r = rigid_motion_from_images_robust(im0, im0, im1, depth0, intrinsic_matrix, intrinsic_matrix, intrinsic_matrix, Array<float>{ 3.f }, Array<float>{ ArrayShape{0} }, fixed_zeros<float, 6>(), false);  // false = estimate_rotation_first
        Array<float> im1t_r = d_pr_bilinear(im0, im1, depth0, intrinsic_matrix, intrinsic_matrix, ke_r);
        draw_nan_masked_rgb(im1t_r, -0.05f, 0.05f).save_to_file("TestOut/rmfi-t-d_pr.png");
        assert_allclose(ke_s.affine(), ke_r.affine());
    }
    {
        TransformationMatrix<float, float, 3> ke = rigid_motion_from_images_robust(im0, im0, im1, depth0, intrinsic_matrix, intrinsic_matrix, intrinsic_matrix, Array<float>{3.f, 1.f, 0.f}, Array<float>{0.1f, 0.05f});
        Array<float> im1t = d_pr_bilinear(im0, im1, depth0, intrinsic_matrix, intrinsic_matrix, ke);
        draw_nan_masked_rgb(im1t, -0.05f, 0.05f).save_to_file("TestOut/rmfi-t-d_pr-3-0.png");

        //for (size_t r = 0; r < depth1.shape(0); ++r) {
        //    for (size_t c = 0; c < depth1.shape(1) / 2; ++c) {
        //        depth0(r, c) *= 1.5;
        //    }
        //}
        Array<float> err = rigid_motion_roundtrip(depth0, depth1, intrinsic_matrix, intrinsic_matrix, ke);
        draw_nan_masked_grayscale(err, 0, 0.5f * 0.5f).save_to_file("TestOut/rmfi-err.png");
    }
}

int main(int argc, char** argv) {
    try {
        // test_jacobian();
        // test_intensity_jacobian();
        test_rigid_motion_from_images();
    } catch (const std::runtime_error& e) {
        lerr() << "ERROR: " << e.what();
        return 1;
    }
    return 0;
}
