#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Sfm/Homography/Homography_From_Images.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <iostream>
#include <random>

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Hfi;

void test_jacobian() {
    // Array<float> intrinsic_matrix = Array<float>::load_txt_2d("Data/camera_intrinsic-256x455.m");
    TransformationMatrix<float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>{
        0.5f, 0.f, 0.9f,
        0.f, 0.7f, 0.2f,
        0.f, 0.f, 1.f} };
    FixedArray<float, 3> theta{ uniform_random_array<float>(ArrayShape{3}, 3) };
    FixedArray<float, 2> x{ uniform_random_array<float>(ArrayShape{2}, 2) };
    FixedArray<float, 2, 3> num = numerical_differentiation<2>([&](const FixedArray<float, 3>& ttheta){
            return transform_coordinates(tait_bryan_angles_2_matrix(ttheta), x, intrinsic_matrix);
        }, theta);
    FixedArray<float, 2, 3> an = projected_points_jacobian_dke_1p_1ke_only_rotation(homogenized_3(x), intrinsic_matrix, theta);
    assert_allclose(num.to_array(), an.to_array(), float(1e-3));
}

void test_intensity_jacobian() {
    TransformationMatrix<float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>{
        0.5f, 0.f, 0.9f,
        0.f, 0.7f, 0.2f,
        0.f, 0.f, 1.f} };
    FixedArray<float, 3> theta{0.2f, 0.1f, 0.4f};
    Array<float> im_r = uniform_random_array<float>(ArrayShape{4, 5}, 1);
    Array<float> im_l = uniform_random_array<float>(ArrayShape{4, 5}, 2);
    Array<float> im_r_f = central_gradient_filter(im_r);
    Array<float> im_l_f = central_gradient_filter(im_l);
    Array<float> ij = intensity_jacobian(im_r_f, im_l_f, intrinsic_matrix, theta);
    assert(all(ij.shape() == ArrayShape{4, 5, 3}));
    assert_isclose(sum(squared(ij)), 966.527f, float(1e-2));

    assert_allclose(
        intensity_jacobian(im_r_f, im_l_f, intrinsic_matrix, theta),
        intensity_jacobian_fast(im_r_f, im_l_f, intrinsic_matrix, theta),
        float{ 1e-4 });
}

void test_homography_from_images() {
    PpmImage im0_raw = PpmImage::load_from_file("Data/rotation-20-256x455x24.ppm");
    PpmImage im1_raw = PpmImage::load_from_file("Data/rotation-40-256x455x24.ppm");
    TransformationMatrix<float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>{ Array<float>::load_txt_2d("Data/camera_intrinsic-256x455.m")} };
    Array<float> im0_rgb = im0_raw.to_float_rgb();
    Array<float> im1_rgb = im1_raw.to_float_rgb();
    Array<float> im0_gray = im0_raw.to_float_grayscale();
    Array<float> im1_gray = im1_raw.to_float_grayscale();
    PpmImage::from_float_rgb(im0_rgb).save_to_file("TestOut/rotation-20.ppm");
    PpmImage::from_float_rgb(im1_rgb).save_to_file("TestOut/rotation-40.ppm");
    PpmImage::from_float_rgb(abs(im0_rgb - im1_rgb)).save_to_file("TestOut/rotation-init-diff.ppm");

    im0_gray = gaussian_filter_NWE(im0_gray, 3.f, NAN);
    im1_gray = gaussian_filter_NWE(im1_gray, 3.f, NAN);

    FixedArray<float, 3, 3> R = rotation_from_images(im0_gray, im1_gray, intrinsic_matrix);
    assert_allclose(R.to_array(), Array<float>{
        {0.999997f, -0.00180258f, -0.00153379f},
        {0.00188353f, 0.998511f, 0.0545234f},
        {0.00143322f, -0.0545261f, 0.998511f}});
    // Array<float> diff = d_pr(im0_gray, im1_gray, intrinsic_matrix, R);
    Array<float> diff = d_pr_bilinear(zeros<float>(im0_gray.shape()), im1_gray, intrinsic_matrix, R);
    draw_nan_masked_grayscale(diff, 0, 0).save_to_file("TestOut/rotation-diff.ppm");
}

int main(int argc, char** argv) {
    try {
        test_jacobian();
        test_intensity_jacobian();
        test_homography_from_images();
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what();
        return 1;
    }
    return 0;
}
