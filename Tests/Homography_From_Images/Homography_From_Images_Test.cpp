#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Sfm/Homography/Homography_From_Images.hpp>
#include <iostream>
#include <random>

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Hfi;

void test_jacobian() {
    // Array<float> intrinsic_matrix = Array<float>::load_txt_2d("Data/camera_intrinsic-256x455.m");
    Array<float> intrinsic_matrix{
        {0.5, 0, 0.9},
        {0, 0.7, 0.2},
        {0, 0, 1}};
    Array<float> theta = random_array4<float>(ArrayShape{3}, 3);
    Array<float> x = random_array4<float>(ArrayShape{2}, 2);
    Array<float> num = numerical_differentiation([&](const Array<float>& ttheta){
            return transform_coordinates(tait_bryan_angles_2_matrix(ttheta), x, intrinsic_matrix);
        }, theta);
    Array<float> an = projected_points_jacobian_dke_1p_1ke_only_rotation(homogenized_3(x), intrinsic_matrix, theta);
    assert_allclose(num, an, float(1e-3));
}

void test_intensity_jacobian() {
    Array<float> intrinsic_matrix{
        {0.5, 0, 0.9},
        {0, 0.7, 0.2},
        {0, 0, 1}};
    Array<float> theta{0.2, 0.1, 0.4};
    Array<float> im_r = random_array4<float>(ArrayShape{4, 5}, 1);
    Array<float> im_l = random_array4<float>(ArrayShape{4, 5}, 2);
    Array<float> im_r_f = central_gradient_filter(im_r);
    Array<float> im_l_f = central_gradient_filter(im_l);
    Array<float> ij = intensity_jacobian(im_r_f, im_l_f, intrinsic_matrix, theta);
    assert(all(ij.shape() == ArrayShape{4, 5, 3}));
    assert_isclose(sum(squared(ij)), 3986.64f, float(1e-2));

    assert_allclose(
        intensity_jacobian(im_r_f, im_l_f, intrinsic_matrix, theta),
        intensity_jacobian_fast(im_r_f, im_l_f, intrinsic_matrix, theta),
        1e-4);
}

void test_homography_from_images() {
    PpmImage im0_raw = PpmImage::load_from_file("Data/rotation-20-256x455x24.ppm");
    PpmImage im1_raw = PpmImage::load_from_file("Data/rotation-40-256x455x24.ppm");
    Array<float> intrinsic_matrix = Array<float>::load_txt_2d("Data/camera_intrinsic-256x455.m");
    Array<float> im0_rgb = im0_raw.to_float_rgb();
    Array<float> im1_rgb = im1_raw.to_float_rgb();
    Array<float> im0_gray = im0_raw.to_float_grayscale();
    Array<float> im1_gray = im1_raw.to_float_grayscale();
    PpmImage::from_float_rgb(im0_rgb).save_to_file("TestOut/rotation-20.ppm");
    PpmImage::from_float_rgb(im1_rgb).save_to_file("TestOut/rotation-40.ppm");
    PpmImage::from_float_rgb(abs(im0_rgb - im1_rgb)).save_to_file("TestOut/rotation-init-diff.ppm");

    im0_gray = gaussian_filter_NWE(im0_gray, 3.f, NAN);
    im1_gray = gaussian_filter_NWE(im1_gray, 3.f, NAN);

    Array<float> R = rotation_from_images(im0_gray, im1_gray, intrinsic_matrix);
    assert_allclose(R, Array<float>{
        {0.999997, -0.00180258, -0.00153379},
        {0.00188353, 0.998511, 0.0545234},
        {0.00143322, -0.0545261, 0.998511}});
    // Array<float> diff = d_pr(im0_gray, im1_gray, intrinsic_matrix, R);
    Array<float> diff = d_pr_bilinear(zeros<float>(im0_gray.shape()), im1_gray, intrinsic_matrix, R);
    draw_nan_masked_grayscale(diff, 0, 0).save_to_file("TestOut/rotation-diff.ppm");
}

int main(int argc, char** argv) {
    test_jacobian();
    test_intensity_jacobian();
    test_homography_from_images();
    return 0;
}
