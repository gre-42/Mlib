#include <Mlib/Assert.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Color_Spaces.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Backward_Differences_Pad_Zeros.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Images/Filters/Difference_Of_Boxes.hpp>
#include <Mlib/Images/Filters/Divide_By_Brightness.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Forward_Differences_Valid.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Filters/Local_Polynomial_Regression.hpp>
#include <Mlib/Images/Filters/Median_Filter.hpp>
#include <Mlib/Images/Filters/Polynomial_Contrast.hpp>
#include <Mlib/Images/Mesh_Coordinates/Meshgrid.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/Quantize.hpp>
#include <Mlib/Images/Resample/Down_Sample_Average.hpp>
#include <Mlib/Images/Resample/Pyramid.hpp>
#include <Mlib/Images/Resample/Up_Sample_Average.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>

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
        Array<float> y = uniform_random_array<float>(ArrayShape{4, 5}, 1);
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
        forward_differences_valid_1d(Array<float>{1,2,0,1,0,0,4}, NAN, 0),
        Array<float>{1, -2, 1, -1, 0, 4, NAN});
    assert_allclose(
        backward_differences_pad_zeros_1d(Array<float>{1,2,0,1,0,0,4}, 0),
        Array<float>{1, 1, -2, 1, -1, 0, 4});
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

void test_harris_nfeatures() {
    const auto bitmap = StbImage3::load_from_file("Data/chessboard1.png");

    const Array<float> image = bitmap.to_float_grayscale();
    StbImage3 res = StbImage3::from_float_grayscale(clipped(80.f * harris_response(image), 0.f, 1.f));
    Array<FixedArray<float, 2>> feature_points = Array<float>::from_dynamic<2>(find_nfeatures(
        harris_response(image),
        ones<bool>(image.shape()),
        20));
    highlight_features(feature_points, res);
    res.save_to_file("TestOut/chessboard1_harris_nfeatures.png");
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
        // lerr() << img;
    });
    assert_isequal(res.size(), nlevels);
    assert_allclose(res.front(), Array<float>{
        {2.5, 5},
        {7.5, 10},
        {12.5, 15},
        {17.5, 20}});
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
            {0, 0.5, 0.333333f, 0.166667f, 0},
            {0, 0, 0, 0, 0}});
    Array<float> res01{
        {0, 0.00382149f, 0.988506f, 0.00382149f, 0},
        {0, 1.47736e-05f, 0.00382149f, 1.47736e-05f, 0}};
    assert_allclose(
        multichannel_gaussian_filter_NWE(Array<float>({im, im}), 0.3f, NAN),
        Array<float>({res01, res01}));
    assert_allclose(
        gaussian_filter_1d_NWE(im, 0.5f, 1, NAN),
        Array<float>{
            {0.000295387f, 0.106479f, 0.786571f, 0.106479f, 0.000295387f},
            {0, 0, 0, 0, 0}});
}

void test_color_spaces() {
    Array<float> a = uniform_random_array<float>(ArrayShape{3, 4, 5}, 1);
    assert_allclose(yuv2rgb(rgb2yuv(a)), a, 1e-5f);
}

void test_central_differences() {
    Array<float> a{1, 1.1f, 2, NAN, 3, 3.3f};
    assert_allclose(
        central_differences_1d(a, 0),
        Array<float>{0.1f, 0.5, 0.9f, 0.5, 0.3f, 0.3f});

    Array<float> b({a, 2.f * a});
    assert_allclose(
        central_gradient_filter(b)[1],
        Array<float>{
            {0.1f, 0.5, 0.9f, 0.5f, 0.3f, 0.3f},
            {0.2f, 1, 1.8f, 1, 0.6f, 0.6f}});

    assert_allclose(
        central_sad_filter(-a),
        Array<float>{0.1f, 0.5f, 0.9f, 0.5f, 0.3f, 0.3f});
}

void test_small_boxes() {
    Array<float> a{1, 1.1f, 2, NAN, 3, 3.3f};
    assert_allclose(
        small_box_filter_NWE(a, ArrayShape{1}, NAN),
        Array<float>{1.05f, 1.36667f, 1.55f, 2.5f, 3.15f, 3.15f},
        1e-5f);

}

void test_difference_of_boxes() {
    Array<float> a{1, 1.1f, 2, NAN, 3, 3.3f};
    assert_allclose(
        difference_of_boxes(a, NAN),
        Array<float>{0.0500001f, 0.266667f, -0.45f, NAN, 0.15f, -0.15f});
}

void test_bilinear_interpolation() {
    Array<float> a{1, 1.1f, 2, NAN, 3, 3.3f};
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
    StbImage3 im = StbImage3::load_from_file("Data/chessboard1.png");
    StbImage3::from_float_rgb(
        clipped(divide_by_brightness(im.to_float_rgb(), 10.f, NAN), 0.f, 1.f))
    .save_to_file("TestOut/chessboard1_dbb.png");
}

void test_down_sample_average() {
    Array<float> a = uniform_random_array<float>(ArrayShape{3, 4}, 1);
    Array<float> b = down_sample_average_1d(a, 1);
    assert_true(all(b.shape() == ArrayShape{3, 2}));
    assert_isclose(b(0, 0), (a(0, 0) + a(0, 1)) / 2);

    Array<float> c = down_sample_average(a);
    assert_true(all(c.shape() == ArrayShape{1, 2}));

    Array<float> d = multichannel_down_sample_average(Array<float>({a, a}));
    assert_true(all(d.shape() == ArrayShape{2, 1, 2}));
}

void test_up_sample_average() {
    Array<float> a = arange<float>(5);
    assert_allclose(
        up_sample_average(a),
        Array<float>{0.f, 0.25f, 0.75f, 1.25f, 1.75f, 2.25f, 2.75f, 3.25f, 3.75f, 4.f});
}

void test_meshgrid() {
    Array<float> mg{ArrayShape{3, 4}};
    meshgrid(mg, 1);
    assert_allclose(mg, Array<float>{
        {0, 1, 2, 3},
        {0, 1, 2, 3},
        {0, 1, 2, 3}});
}

void test_local_polynomial_regression() {
    Array<float> image(random_array3<float>(ArrayShape{5, 6}, 1));
    meshgrid(image, 0);
    // lerr() << local_polynomial_regression(image, [](const Array<float>& im){return gaussian_filter_NWE(im, 1.f, NAN, 4.f, false);}, 2);
}

void test_waveform() {
    std::ofstream ofstr{ "TestOut/waveform.svg" };
    Svg<float> svg{ ofstr, 200, 100 };
    std::vector<float> x{1.f, 3.f, 5.f, 6.f};
    std::vector<bool> y0{0, 0, 1, 0};
    std::vector<bool> y1{0, 1, 0, 0};
    svg.plot_waveforms(x, {y0, y1}, {"a", "b"}, 1.f, {"#000", "#FF5733"}, 1);
    svg.finish();
}

void test_polynomial_contrast() {
    // {
    //     size_t poly_degree = 2;
    //     Array<float> weights{ 0.5f, 0.6f, 1.5f, 0.5f, 0.4f };
    //     Array<float> contrast = zeros<float>(ArrayShape{ 1 + poly_degree });
    //     contrast(0) = 1;
    //     Array<Array<float>> A{ ArrayShape{ 1 }};
    //     A[0] = linspace<float>(-1, 1, weights.length());
    //     weights = polynomial_contrast(A, weights, contrast, poly_degree);
    //     lerr() << weights;
    // }
    auto w = [](const Array<double>& d){return gaussian_filter_NWE(d, 1.23, double{NAN}, 4., FilterExtension::NONE);};
    auto l = [&w](const Array<double>& d){return local_polynomial_regression(d, w, 2);};

    assert_allclose(
        gaussian_filter_NWE(dirac_array<double>(ArrayShape{41}, ArrayShape{20}), 1.23, double{NAN}, 4., FilterExtension::NONE, 2),
        l(dirac_array<double>(ArrayShape{41}, ArrayShape{20})));
}

int main(int argc, char **argv) {
    enable_floating_point_exceptions();

    try {
        test_up_sample2();
        test_down_sample_average();
        test_up_sample_average();
        test_waveform();
        test_differences();
        test_forward_backward_differences();
        test_laplace();
        test_harris_nfeatures();
        test_pyramid();
        test_quantize();
        test_median_filter_2d();
        test_lowpass();
        test_color_spaces();
        test_central_differences();
        test_small_boxes();
        test_difference_of_boxes();
        test_bilinear_interpolation();
        test_division_by_brightness();
        test_meshgrid();
        test_local_polynomial_regression();
        test_polynomial_contrast();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
