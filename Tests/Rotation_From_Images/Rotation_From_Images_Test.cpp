#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Math/Fixed_Test.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rotation_From_Images.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <iostream>
#include <random>

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Hfi;

void test_jacobian() {
    // Array<float> intrinsic_matrix = Array<float>::load_txt_2d("Data/camera_intrinsic-256x455.m");
    TransformationMatrix<float, float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>::init(
        0.5f, 0.f, 0.9f,
        0.f, 0.7f, 0.2f,
        0.f, 0.f, 1.f) };
    FixedArray<float, 3> theta{ uniform_random_array<float>(ArrayShape{3}, 3) };
    FixedArray<float, 2> x{ uniform_random_array<float>(ArrayShape{2}, 2) };
    FixedArray<float, 2, 3> num = numerical_differentiation<2>([&](const FixedArray<float, 3>& ttheta){
            return transform_coordinates(tait_bryan_angles_2_matrix(ttheta), x, intrinsic_matrix, intrinsic_matrix);
        }, theta);
    FixedArray<float, 2, 3> an = projected_points_jacobian_dke_1p_1ke_only_rotation(homogenized_3(x), intrinsic_matrix, intrinsic_matrix, theta);
    assert_allclose(num, an, float(1e-2));
}

void test_intensity_jacobian() {
    TransformationMatrix<float, float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>::init(
        0.5f, 0.f, 0.9f,
        0.f, 0.7f, 0.2f,
        0.f, 0.f, 1.f) };
    FixedArray<float, 3> theta{0.2f, 0.1f, 0.4f};
    Array<float> im_r = uniform_random_array<float>(ArrayShape{2, 4, 5}, 1);
    Array<float> im_l = uniform_random_array<float>(ArrayShape{2, 4, 5}, 2);
    Array<float> im_r_f = multichannel_central_gradient_filter(im_r);
    Array<float> im_l_f = multichannel_central_gradient_filter(im_l);
    Array<float> ij = intensity_jacobian(im_r_f, im_l_f, intrinsic_matrix, intrinsic_matrix, theta);
    assert(all(ij.shape() == ArrayShape{2, 4, 5, 3}));
    assert_isclose(sum(squared(ij)), 29428.1f, float(1e-1));

    assert_allclose(
        intensity_jacobian(im_r_f, im_l_f, intrinsic_matrix, intrinsic_matrix, theta),
        intensity_jacobian_fast(im_r_f, im_l_f, intrinsic_matrix, intrinsic_matrix, theta),
        float{ 1e-3 });
}

void test_homography_from_images() {
    StbImage3 im0_raw = StbImage3::load_from_file("Data/rotation-20-256x455x24.png");
    StbImage3 im1_raw = StbImage3::load_from_file("Data/rotation-40-256x455x24.png");
    TransformationMatrix<float, float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>{ Array<float>::load_txt_2d("Data/camera_intrinsic-256x455.m")} };
    Array<float> im0_rgb = im0_raw.to_float_rgb();
    Array<float> im1_rgb = im1_raw.to_float_rgb();
    StbImage3::from_float_rgb(im0_rgb).save_to_file("TestOut/rotation-20.png");
    StbImage3::from_float_rgb(im1_rgb).save_to_file("TestOut/rotation-40.png");
    StbImage3::from_float_rgb(abs(im0_rgb - im1_rgb)).save_to_file("TestOut/rotation-init-diff.png");

    im0_rgb = multichannel_gaussian_filter_NWE(im0_rgb, 3.f, NAN);
    im1_rgb = multichannel_gaussian_filter_NWE(im1_rgb, 3.f, NAN);

    FixedArray<float, 3, 3> R = rotation_from_images(im0_rgb, im1_rgb, intrinsic_matrix, intrinsic_matrix);
    assert_allclose(R, FixedArray<float, 3, 3>::init(
        0.999998f, -0.00143413f, -0.00139267f,
        0.00150745f, 0.998531f, 0.0541625f,
        0.00131294f, -0.0541645f, 0.998531f));
    // Array<float> diff = d_pr(im0_rgb, im1_rgb, intrinsic_matrix, R);
    Array<float> diff = d_pr_bilinear(zeros<float>(im0_rgb.shape()), im1_rgb, intrinsic_matrix, intrinsic_matrix, R);
    draw_nan_masked_rgb(diff, 0, 0).save_to_file("TestOut/rotation-diff.png");
}

int main(int argc, char** argv) {
    try {
        test_jacobian();
        test_intensity_jacobian();
        test_homography_from_images();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
