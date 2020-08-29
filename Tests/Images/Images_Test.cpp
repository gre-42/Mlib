#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Color_Spaces.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Backward_Differences.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Images/Filters/Difference_Of_Boxes.hpp>
#include <Mlib/Images/Filters/Divide_By_Brightness.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Forward_Differences.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Filters/Median_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Images/Quantize.hpp>
#include <Mlib/Images/Resample/Down_Sample_Average.hpp>
#include <Mlib/Images/Resample/Pyramid.hpp>
#include <fenv.h>

using namespace Mlib;

void test_differences() {
    assert_allclose(
        difference_filter_1d(Array<float>{0,0,0,1,0,0,0}, NAN, 0),
        Array<float>{NAN, 0, 0.5, 0, -0.5, 0, NAN});
    // zeros at the boundary (2D)
    assert_allclose(
        sad_filter(Array<float>{
            {0,0,0,0,0,0,0},
            {0,0,0,1,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0}}, NAN),
        Array<float>{
            {NAN, NAN, NAN, NAN, NAN, NAN, NAN},
            {NAN, 0, 0.25, 0, 0.25, 0, NAN},
            {NAN, 0, 0, 0.25, 0, 0, NAN},
            {NAN, NAN, NAN, NAN, NAN, NAN, NAN}});
    {
        Array<float> y = random_array4<float>(ArrayShape{4, 5}, 1);
        assert_allclose(
            difference_filter_1d(y, NAN, 0),
            gradient_filter(y, NAN)[0]);
        assert_allclose(
            difference_filter_1d(y, NAN, 1),
            gradient_filter(y, NAN)[1]);
    }
}

void test_forward_backward_differences() {
    assert_allclose(
        forward_differences_1d(Array<float>{1,2,0,1,0,0,4}, NAN, 0),
        Array<float>{1, -2, 1, -1, 0, 4, NAN});
    assert_allclose(
        backward_differences_1d(Array<float>{1,2,0,1,0,0,4}, NAN, 0),
        Array<float>{NAN, 1, -2, 1, -1, 0, 4});
}

void test_laplace() {
    assert_allclose(
        laplace_filter(Array<float>{0,0,0,1,0,0,0}, NAN),
        Array<float>{NAN, 0, 1, -2, 1, 0, NAN});
    // values at the boundary (2D)
    // laplace=addition, not separability
    assert_allclose(
        laplace_filter(Array<float>{
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,1,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0}}, NAN),
        Array<float>{
            {NAN, NAN, NAN, NAN, NAN, NAN, NAN},
            {NAN, 0, 0, 1, 0, 0, NAN},
            {NAN, 0, 1, -4, 1, 0, NAN},
            {NAN, 0, 0, 1, 0, 0, NAN},
            {NAN, NAN, NAN, NAN, NAN, NAN, NAN}});
}

void test_harris_response_array() {
    auto pic3x3 = Array<float>{
        {0,0,0,0,0},
        {0,0,1,0,0},
        {0,0,0,0,0}};
    assert_allclose(
        harris_response(pic3x3),
        Array<float>{
            {0,0,-0.05,0,0},
            {0,-.003125,0,-.003125,0},
            {0,0,-0.05,0,0}});
    assert_allclose<float>(
        find_harris_corners(pic3x3, -0.001),
        Array<float>{{1.5, 1.5}, {3.5, 1.5}});
}

void test_harris_response() {
    const auto bitmap = PpmImage::load_from_file("Data/chessboard1.ppm");

    const Array<float> image = bitmap.to_float_grayscale();
    PpmImage res = PpmImage::from_float_grayscale(clipped(-80.f * harris_response(image), 0.f, 1.f));
    // std::cerr << min(harris_response(image)) << std::endl;
    // std::cerr << max(harris_response(image)) << std::endl;
    Array<float> feature_points = find_harris_corners(image);
    highlight_features(feature_points, res);
    res.save_to_file("TestOut/chessboard1_harris_response.ppm");
}

void test_harris_nfeatures() {
    const auto bitmap = PpmImage::load_from_file("Data/chessboard1.ppm");

    const Array<float> image = bitmap.to_float_grayscale();
    PpmImage res = PpmImage::from_float_grayscale(clipped(-80.f * harris_response(image), 0.f, 1.f));
    Array<float> feature_points = find_nfeatures(
        -harris_response(image),
        ones<bool>(image.shape()),
        20);
    highlight_features(feature_points, res);
    res.save_to_file("TestOut/chessboard1_harris_nfeatures.ppm");
}

void test_pyramid() {
    Array<float> images{
        {1, 2, 3, 4, 5},
        {6, 7, 8, 9, 10},
        {11, 12, 13, 14, 15},
        {16, 17, 18, 19, 20}};
    size_t nlevels = 3;
    size_t reduction = 2;
    std::list<Array<float>> res;
    resampling_pyramid(images, nlevels, reduction, [&](const Array<float>& img) -> void {
        res.push_back(img);
        // std::cerr << img << std::endl;
    });
    assert_isclose<float>(res.size(), nlevels);
    assert_allclose(res.front(), Array<float>{
        {2.5, NAN},
        {7.5, NAN},
        {12.5, NAN},
        {17.5, NAN}});
    assert_allclose(res.back(), images);
}

void test_quantize() {
    {
        Array<float> a{1, 2, 3, 7, 8, 9};
        assert_allclose(quantized(a, Array<float>{2, 8}), Array<float>{2, 2, 2, 8, 8, 8});
    }

    {
        Array<float> a{1, 2, 3, 7, 8, NAN, 9};
        assert_allclose(quantized(a, Array<float>{2, 8}), Array<float>{2, 2, 2, 8, 8, NAN, 8});
    }
}

void test_median_filter_2d() {
    Array<float> im{
        {2, 2, 2, 2, 3, 3, 3},
        {2, 2, 10, 2, 3, 3, 3},
        {2, 2, 2, 2, 3, 3, 3},
        {2, 2, 2, 2, 3, 3, 3},
        {2, 2, 2, 2, 3, 3, 3}};
    assert_allclose(median_filter_2d(im, 0), im);
    assert_allclose(
        median_filter_2d(im, 1),
        Array<float>{
        {NAN, NAN, NAN, NAN, NAN, NAN, NAN},
        {NAN, 2, 2, 2, 3, 3, NAN},
        {NAN, 2, 2, 2, 3, 3, NAN},
        {NAN, 2, 2, 2, 3, 3, NAN},
        {NAN, NAN, NAN, NAN, NAN, NAN, NAN}});
}

void test_up_sample2() {
    Array<float> im{
        {2, 3, 4, 5},
        {3, 4, 5, 6}};
    assert_allclose(
        up_sample2(im),
        Array<float>{
            {2, 2.5, 3, 3.5, 4, 4.5, 5},
            {2.5, 3, 3.5, 4, 4.5, 5, 5.5},
            {3, 3.5, 4, 4.5, 5, 5.5, 6}});
    assert_allclose(down_sample2(up_sample2(im)), im);
}

void test_lowpass() {
    Array<float> im{
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0}};
    assert_allclose(
        lowpass_filter_1d_NWE(im, Array<float>{1, 2, 3}, NAN, 1),
        Array<float>{
            {0, 0.5, 0.333333, 0.166667, 0},
            {0, 0, 0, 0, 0}});
    Array<float> res01{
        {0, 0.00382149, 0.988506, 0.00382149, 0},
        {0, 1.47736e-05, 0.00382149, 1.47736e-05, 0}};
    assert_allclose(
        multichannel_gaussian_filter_NWE(Array<float>({im, im}), 0.3f, NAN),
        Array<float>({res01, res01}));
    assert_allclose(
        gaussian_filter_1d_NWE(im, 0.5f, 1, NAN),
        Array<float>{
            {0.000295387, 0.106479, 0.786571, 0.106479, 0.000295387},
            {0, 0, 0, 0, 0}});
}

void test_color_spaces() {
    Array<float> a = random_array4<float>(ArrayShape{3, 4, 5}, 1);
    assert_allclose(yuv2rgb(rgb2yuv(a)), a, 1e-5);
}

void test_central_differences() {
    Array<float> a{1, 1.1, 2, NAN, 3, 3.3};
    assert_allclose(
        central_differences_1d(a, 0),
        Array<float>{0.1, 0.5, 0.9, 0.5, 0.3, 0.3});

    Array<float> b({a, 2.f * a});
    assert_allclose(
        central_gradient_filter(b)[1],
        Array<float>{
            {0.1, 0.5, 0.9, 0.5, 0.3, 0.3},
            {0.2, 1, 1.8, 1, 0.6, 0.6}});

    assert_allclose(
        central_sad_filter(-a),
        Array<float>{0.1, 0.5, 0.9, 0.5, 0.3, 0.3});
}

void test_small_boxes() {
    Array<float> a{1, 1.1, 2, NAN, 3, 3.3};
    assert_allclose(
        small_box_filter_NWE(a, ArrayShape{1}, NAN),
        Array<float>{1.05, 1.36667, 1.55, 2.5, 3.15, 3.15},
        1e-5);

}

void test_difference_of_boxes() {
    Array<float> a{1, 1.1, 2, NAN, 3, 3.3};
    assert_allclose(
        difference_of_boxes(a, NAN),
        Array<float>{0.0500001, 0.266667, -0.45, NAN, 0.15, -0.15});
}

void test_bilinear_interpolation() {
    Array<float> a{1, 1.1, 2, NAN, 3, 3.3};
    Array<float> b({a, 2.f * a});
    float intensity;
    assert_true(bilinear_grayscale_interpolation(0.f, 0.f, b, intensity));
    assert_isclose(intensity, 1.f);
    assert_true(bilinear_grayscale_interpolation(0.f, 1.f, b, intensity));
    assert_isclose(intensity, 1.1f);
    assert_true(bilinear_grayscale_interpolation(1.f, 1.f, b, intensity));
    assert_isclose(intensity, 2.2f);
    assert_true(bilinear_grayscale_interpolation(1.f, .5f, b, intensity));
    assert_isclose(intensity, 2.1f);
}

void test_division_by_brightness() {
    PpmImage im = PpmImage::load_from_file("Data/chessboard1.ppm");
    PpmImage::from_float_rgb(
        clipped(divide_by_brightness(im.to_float_rgb(), 10.f, NAN), 0.f, 1.f))
    .save_to_file("TestOut/chessboard1_dbb.ppm");
}

void test_down_sample_average() {
    Array<float> a = random_array4<float>(ArrayShape{3, 4}, 1);
    Array<float> b = down_sample_average_1d(a, 1);
    assert_true(all(b.shape() == ArrayShape{3, 2}));
    assert_isclose(b(0, 0), (a(0, 0) + a(0, 1)) / 2);

    Array<float> c = down_sample_average(a);
    assert_true(all(c.shape() == ArrayShape{1, 2}));

    Array<float> d = multichannel_down_sample_average(Array<float>({a, a}));
    assert_true(all(d.shape() == ArrayShape{2, 1, 2}));
}

int main(int argc, char **argv) {
    #ifndef __MINGW32__
    feenableexcept(FE_INVALID);
    #endif

    test_differences();
    test_forward_backward_differences();
    test_laplace();
    test_harris_response_array();
    test_harris_response();
    test_harris_nfeatures();
    test_pyramid();
    test_quantize();
    test_median_filter_2d();
    test_up_sample2();
    test_lowpass();
    test_color_spaces();
    test_central_differences();
    test_small_boxes();
    test_difference_of_boxes();
    test_bilinear_interpolation();
    test_division_by_brightness();
    test_down_sample_average();
    return 0;
}
