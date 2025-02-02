#include <Mlib/Assert.hpp>
#include <Mlib/Images/Bgr24Raw.hpp>
#include <Mlib/Images/Bgr565Bitmap.hpp>
#include <Mlib/Images/Pgm_Image.hpp>
#include <Mlib/Images/Ppm_Image.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <iostream>

using namespace Mlib;

void bmp_test() {
    auto bitmap = Bgr565Bitmap::load_from_file("Data/test565.bmp");
    // lerr() << "shape " << bitmap.shape();
    assert_true(bitmap.shape(0) == 48);
    assert_true(bitmap.shape(1) == 100);
    assert_true(&bitmap(2, 0) - &bitmap(1, 0) == 100);
    bitmap(2, 0).r = 0x0;
    bitmap(2, 0).g = 0x0;
    bitmap(2, 0).b = 0x0;
    bitmap.save_to_file("TestOut/test565_1.bmp");
}

void bmp_no_power_of_2_test() {
    Bgr565Bitmap bmp{ArrayShape{256, 257}, Bgr565::white()};
    bmp.draw_fill_rect(FixedArray<size_t, 2>{ (size_t)60, (size_t)50 }, 5, Bgr565::blue());
    bmp.save_to_file("TestOut/test_no_power_of_2_test.bmp");

    Bgr565Bitmap bmp1 = Bgr565Bitmap::load_from_file("TestOut/test_no_power_of_2_test.bmp");
    bmp1.save_to_file("TestOut/test_no_power_of_2_test_2.bmp");
}

void raw_test() {
    Bgr565Bitmap::from_float_grayscale(
        Bgr24Raw::load_from_file("Data/box-32x40x24.bgr")
        .to_float_grayscale()).save_to_file("TestOut/box-32x40x24.bmp");
}

void ppm_test() {
    PpmImage::from_float_grayscale(
        Bgr24Raw::load_from_file("Data/box-32x40x24.bgr")
        .to_float_grayscale()).save_to_file("TestOut/box-32x40x24.ppm");
    Array<float> x = linspace(0.f, 1.f, 200);
    Array<float> x2({x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x});
    PpmImage::from_float_grayscale(x2).save_to_file("TestOut/linspace.ppm");
    Bgr565Bitmap::from_float_grayscale(x2).save_to_file("TestOut/linspace.bmp");
    PpmImage::from_float_grayscale(x2.T()).save_to_file("TestOut/linspace-y.ppm");
    Bgr565Bitmap::from_float_grayscale(x2.T()).save_to_file("TestOut/linspace-y.bmp");

    PpmImage::load_from_file("TestOut/box-32x40x24.ppm")
    .save_to_file("TestOut/box-32x40x24-2.ppm");
}

void pgm_test() {
    auto im = PgmImage::from_float(uniform_random_array<float>(ArrayShape{5, 6}, 1));
    im.save_to_file("TestOut/rpgm.pgm");
    auto ld = PgmImage::load_from_file("TestOut/rpgm.pgm");
    assert_allequal(im, ld);
}

int main(int argc, char **argv) {
    bmp_test();
    bmp_no_power_of_2_test();
    raw_test();
    ppm_test();
    pgm_test();
    return 0;
}
