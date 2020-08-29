#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Math/Math.hpp>

using namespace Mlib;

void test_box_filter_1d_dirac() {
    Array<float> image(ArrayShape{5});
    image = 0;
    image(2) = 1;
    Array<float> filtered1 = box_filter_1d(image, 3, 0.f, 0);
    assert(all(filtered1.shape() == image.shape()));
    const float f13 = 1. / 3;
    assert_allclose(filtered1, Array<float>{0, f13, f13, f13, 0});

    Array<float> filtered2 = box_filter(image, ArrayShape{3}, 0.f);
    assert(all(filtered1 == filtered2));
}

void test_box_filter_1d() {
    Array<float> image(ArrayShape{6});
    image = 0;
    image(2) = 1;
    image(3) = 1;
    Array<float> filtered1 = box_filter_1d(image, 3, 0.f, 0);
    // std::cerr << filtered1 << std::endl;
    assert(all(filtered1.shape() == image.shape()));
    const float f13 = 1. / 3;
    assert_allclose(filtered1, Array<float>{0, f13, 2 * f13, 2 * f13, f13, 0});

    Array<float> filtered2 = box_filter(image, ArrayShape{3}, 0.f);
    assert(all(filtered1 == filtered2));
}

void test_box_filter_2d_dirac() {
    Array<float> image(ArrayShape{5, 6});
    image = 0;
    image(2, 2) = 1;
    ArrayShape box_shape{3, 3};
    Array<float> filtered = box_filter(image, box_shape, 0.f);
    assert(all(filtered.shape() == image.shape()));
    const float f9 = 1. / 9;
    assert_allclose(filtered, Array<float>{
        {0, 0, 0, 0, 0, 0},
        {0, f9, f9, f9, 0, 0},
        {0, f9, f9, f9, 0, 0},
        {0, f9, f9, f9, 0, 0},
        {0, 0, 0, 0, 0, 0}});
}

void test_box_filter_full() {
    {
        Array<float> image = ones<float>(ArrayShape{4, 5});
        Array<float> filtered = box_filter(image, ArrayShape{3, 3}, 0.f);
        assert_allclose(filtered, Array<float>{
            {0, 0, 0, 0, 0},
            {0, 1, 1, 1, 0},
            {0, 1, 1, 1, 0},
            {0, 0, 0, 0, 0}});
    }

    {
        Array<float> image = ones<float>(ArrayShape{4, 3});
        Array<float> filtered = box_filter(image, ArrayShape{1, 1}, 0.f);
        assert_allclose(filtered, Array<float>{
            {1, 1, 1},
            {1, 1, 1},
            {1, 1, 1},
            {1, 1, 1}});
    }

    {
        Array<float> image = ones<float>(ArrayShape{9, 7});
        Array<float> filtered = box_filter(image, ArrayShape{3, 4}, 0.f);
        assert_allclose(filtered, Array<float>{
            {0, 0, 0, 0, 0, 0, 0},
            {0, 1, 1, 1, 1, 0, 0},
            {0, 1, 1, 1, 1, 0, 0},
            {0, 1, 1, 1, 1, 0, 0},
            {0, 1, 1, 1, 1, 0, 0},
            {0, 1, 1, 1, 1, 0, 0},
            {0, 1, 1, 1, 1, 0, 0},
            {0, 1, 1, 1, 1, 0, 0},
            {0, 0, 0, 0, 0, 0, 0}});
    }

    {
        Array<float> image = ones<float>(ArrayShape{8}) * 1.5f;
        Array<float> filtered = box_filter(image, ArrayShape{6}, 0.f);
        assert_allclose(filtered, Array<float>{0, 0, 1.5, 1.5, 1.5, 0, 0, 0});
    }
}

void test_box_filter_nan() {
    Array<float> image{
        {NAN, NAN, NAN, NAN, NAN},
        {NAN, 0, 0.5, 0.5, NAN},
        {NAN, 0, 0.5, 0.5, NAN},
        {NAN, 0, 0.5, 0.5, NAN},
        {NAN, NAN, NAN, NAN, NAN}};
    assert_allclose(
        box_filter_nan(image, ArrayShape{3, 3}, NAN),
        Array<float>{
        {NAN, NAN, NAN, NAN, NAN},
        {NAN, NAN, NAN, NAN, NAN},
        {NAN, NAN, 1./3, NAN, NAN},
        {NAN, NAN, NAN, NAN, NAN},
        {NAN, NAN, NAN, NAN, NAN}});
}

int main(int argc, char** argv) {
    test_box_filter_1d_dirac();
    test_box_filter_2d_dirac();
    test_box_filter_1d();
    test_box_filter_full();
    test_box_filter_nan();
    return 0;
}
