#include <Mlib/Images/Filters/Box_Filter.hpp>
#include <Mlib/Images/Filters/Small_Box_Filter.hpp>
#include <Mlib/Math/Math.hpp>

using namespace Mlib;

void test_box_filter_1d_dirac() {
    Array<float> image(ArrayShape{5});
    image = 0;
    image(2) = 1;
    Array<float> filtered1 = box_filter_append_zeros_1d(image, 3, 0);
    assert(all(filtered1.shape() == image.shape()));
    const float f13 = 1.f / 3;
    assert_allclose(filtered1, Array<float>{0, f13, f13, f13, 0});

    Array<float> filtered2 = box_filter_append_zeros(image, ArrayShape{3});
    assert(all(filtered1 == filtered2));
}

void test_box_filter_1d() {
    Array<float> image(ArrayShape{6});
    image = 0;
    image(2) = 1;
    image(3) = 1;
    Array<float> filtered1 = box_filter_append_zeros_1d(image, 3, 0);
    // lerr() << filtered1;
    assert(all(filtered1.shape() == image.shape()));
    const float f13 = 1.f / 3;
    assert_allclose(filtered1, Array<float>{0, f13, 2 * f13, 2 * f13, f13, 0});

    Array<float> filtered2 = box_filter_append_zeros(image, ArrayShape{3});
    assert(all(filtered1 == filtered2));
}

void test_box_filter_2d_dirac() {
    Array<float> image(ArrayShape{5, 6});
    image = 0;
    image(2, 2) = 1;
    ArrayShape box_shape{3, 3};
    Array<float> filtered = box_filter_append_zeros(image, box_shape);
    assert(all(filtered.shape() == image.shape()));
    const float f9 = 1.f / 9;
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
        Array<float> filtered = box_filter_append_zeros(image, ArrayShape{3, 3});
        float f4 = 4.f / 9;
        float f6 = 2.f / 3;
        assert_allclose(filtered, Array<float>{
            { f4, f6, f6, f6, f4},
            { f6, 1., 1., 1., f6},
            { f6, 1., 1., 1., f6},
            { f4, f6, f6, f6, f4 }});
    }

    {
        Array<float> image = ones<float>(ArrayShape{4, 3});
        Array<float> filtered = box_filter_append_zeros(image, ArrayShape{1, 1});
        assert_allclose(filtered, Array<float>{
            {1, 1, 1},
            {1, 1, 1},
            {1, 1, 1},
            {1, 1, 1}});
    }

    {
        Array<float> image = ones<float>(ArrayShape{9, 7});
        Array<float> filtered = box_filter_append_zeros(image, ArrayShape{3, 4});
        float f3 = 1.f / 3;
        float f6 = 2.f / 3;
        assert_allclose(filtered, Array<float>{
            {.5, f6, f6, f6, f6, .5, f3},
            {.75, 1., 1., 1., 1., .75, .5},
            {.75, 1., 1., 1., 1., .75, .5},
            {.75, 1., 1., 1., 1., .75, .5},
            {.75, 1., 1., 1., 1., .75, .5},
            {.75, 1., 1., 1., 1., .75, .5},
            {.75, 1., 1., 1., 1., .75, .5},
            {.75, 1., 1., 1., 1., .75, .5},
            {.5, f6, f6, f6, f6, .5, f3}});
    }

    {
        Array<float> image = ones<float>(ArrayShape{8}) * 1.5f;
        Array<float> filtered = box_filter_append_zeros(image, ArrayShape{6});
        assert_allclose(filtered, Array<float>{1, 1.25, 1.5, 1.5, 1.5, 1.25, 1, 0.75});
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
        {NAN, NAN, 1.f/3, NAN, NAN},
        {NAN, NAN, NAN, NAN, NAN},
        {NAN, NAN, NAN, NAN, NAN}});
}

void test_box_filter_rand() {
    for (size_t r = 1; r < 9; ++r) {
        for (size_t c = 1; c < 9; ++c) {
            for (size_t sr = 0; sr < 4; ++sr) {
                for (size_t sc = 0; sc < 4; ++sc) {
                    Array<float> ra = random_array3<float>(ArrayShape{ r, c }, 1);
                    Array<float> rb = box_filter_nans_as_zeros_NWE(ra, ArrayShape{ 1 + 2 * sr, 1 + 2 * sc });
                    Array<float> rc = small_box_filter_NWE(ra, ArrayShape{ sr, sc }, NAN);
                    assert_allclose(rb, rc);
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    try {
        test_box_filter_1d_dirac();
        test_box_filter_2d_dirac();
        test_box_filter_1d();
        test_box_filter_full();
        test_box_filter_nan();
        test_box_filter_rand();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
