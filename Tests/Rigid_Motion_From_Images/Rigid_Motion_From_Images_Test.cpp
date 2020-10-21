#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Bgr565Bitmap.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Sfm/Data_Generators/Folder_Data_Generator.hpp>
#include <Mlib/Sfm/Pipelines/Reconstruction/Template_Patch_Pipeline.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images_Robust.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images_Smooth.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_Roundtrip.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <iostream>
#include <random>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Rmfi;

void test_jacobian() {
    Array<float> intrinsic_matrix{
        {0.5, 0, 0.9},
        {0, 0.7, 0.2},
        {0, 0, 1}};
    Array<float> kep = uniform_random_array<float>(ArrayShape{6}, 3);
    Array<float> x = uniform_random_array<float>(ArrayShape{2}, 2);
    float depth = 2.345;
    Array<float> num = numerical_differentiation([&](const Array<float>& kkep){
            return transform_coordinates(intrinsic_matrix, k_external(kkep), x, depth);
        }, kep);
    Array<float> an = projected_points_jacobian_dke_1p_1ke_lifting(x, depth, intrinsic_matrix, kep);
    assert_allclose(num, an, float(1e-3));
}

void test_intensity_jacobian() {
    Array<float> intrinsic_matrix{
        {0.5, 0, 0.9},
        {0, 0.7, 0.2},
        {0, 0, 1}};
    Array<float> kep{0.2, 0.1, 0.4, 0.12, 0.34, 0.56};
    Array<float> im_r = uniform_random_array<float>(ArrayShape{4, 5}, 1);
    Array<float> im_l = uniform_random_array<float>(ArrayShape{4, 5}, 2);
    Array<float> im_r_depth = uniform_random_array<float>(ArrayShape{4, 5}, 1) * 0.2f + 2.f;
    Array<float> im_r_f = central_gradient_filter(im_r);
    Array<float> im_l_f = central_gradient_filter(im_l);
    Array<float> ij = intensity_jacobian(im_r_f, im_l_f, im_r_depth, intrinsic_matrix, kep);
    assert(all(ij.shape() == ArrayShape{4, 5, 6}));
    assert_isclose(sum(squared(ij)), 611.727f, float(1e-2));

    assert_allclose(
        intensity_jacobian(im_r_f, im_l_f, im_r_depth, intrinsic_matrix, kep),
        intensity_jacobian_fast(im_r_f, im_l_f, im_r_depth, intrinsic_matrix, kep),
        1e-4);
}

void test_rigid_motion_from_images() {
    Array<float> intrinsic_matrix = Array<float>::load_txt_2d("Data/camera_intrinsic-256x455.m");
    Array<float> depth0 = Array<float>::load_binary("Data/Rigid_Motion/depth-0-590.array");
    Array<float> depth1 = Array<float>::load_binary("Data/Rigid_Motion/depth-200-790.array");
    Array<float> im0 = PpmImage::load_from_file("Data/Rigid_Motion/vid001-256x455x24.ppm").to_float_grayscale();
    Array<float> im1 = PpmImage::load_from_file("Data/Rigid_Motion/vid021-256x455x24.ppm").to_float_grayscale();

    draw_nan_masked_grayscale(im0, 0, 1).save_to_file("TestOut/rmfi-0.ppm");
    draw_nan_masked_grayscale(im1, 0, 1).save_to_file("TestOut/rmfi-1.ppm");
    Array<float> im0m = d_pr_bilinear(im0, im1, depth0, intrinsic_matrix, assemble_inverse_homogeneous_3x4(identity_array<float>(3), zeros<float>(ArrayShape{3})));
    draw_nan_masked_grayscale(im0m, -0.1, 0.1).save_to_file("TestOut/rmfi-d_pr.ppm");

    {
        Array<float> ke_id = rigid_motion_from_images_smooth(im0, im0, depth0, intrinsic_matrix, {1.f});
        assert_allclose(ke_id, identity_array<float>(4).row_range(0, 3));
    }
    {
        Array<float> ke_s = rigid_motion_from_images_smooth(im0, im1, depth0, intrinsic_matrix, {3.f});
        Array<float> im1t_s = d_pr_bilinear(im0, im1, depth0, intrinsic_matrix, ke_s);

        Array<float> ke_r = rigid_motion_from_images_robust(im0, im0, im1, depth0, intrinsic_matrix, {3.f}, {}, zeros<float>(ArrayShape{6}), false);  // false = estimate_rotation_first
        Array<float> im1t_r = d_pr_bilinear(im0, im1, depth0, intrinsic_matrix, ke_r);
        draw_nan_masked_grayscale(im1t_r, -0.05, 0.05).save_to_file("TestOut/rmfi-t-d_pr.ppm");
        assert_allclose(ke_s, ke_r);
    }
    {
        Array<float> ke = rigid_motion_from_images_robust(im0, im0, im1, depth0, intrinsic_matrix, {3.f, 1.f, 0.f}, {0.1f, 0.05});
        Array<float> im1t = d_pr_bilinear(im0, im1, depth0, intrinsic_matrix, ke);
        draw_nan_masked_grayscale(im1t, -0.05, 0.05).save_to_file("TestOut/rmfi-t-d_pr-3-0.ppm");

        //for(size_t r = 0; r < depth1.shape(0); ++r) {
        //    for(size_t c = 0; c < depth1.shape(1) / 2; ++c) {
        //        depth0(r, c) *= 1.5;
        //    }
        //}
        Array<float> err = rigid_motion_roundtrip(depth0, depth1, intrinsic_matrix, ke);
        draw_nan_masked_grayscale(err, 0, 0.5 * 0.5).save_to_file("TestOut/rmfi-err.ppm");
    }
}

int main(int argc, char** argv) {
    test_jacobian();
    test_intensity_jacobian();
    test_rigid_motion_from_images();
    return 0;
}
