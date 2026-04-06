#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Color_Spaces.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Filters/Local_Polynomial_Regression.hpp>
#include <Mlib/Images/Filters/Median_Filter.hpp>
#include <Mlib/Images/Filters/Polynomial_Contrast.hpp>
#include <Mlib/Images/Filters/Small_Box_Filter.hpp>
#include <Mlib/Images/Mesh_Coordinates/Meshgrid.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Images/Transform/Downsample.hpp>
#include <Mlib/Misc/Floating_Point_Exceptions.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <Mlib/Testing/Assert.hpp>

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
        test_waveform();
        test_differences();
        test_laplace();
        test_median_filter_2d();
        test_lowpass();
        test_color_spaces();
        test_central_differences();
        test_small_boxes();
        test_bilinear_interpolation();
        test_meshgrid();
        test_local_polynomial_regression();
        test_polynomial_contrast();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
