#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <iostream>

using namespace Mlib;

void test_saddle_detector(const std::string& basename) {
    auto bitmap = StbImage3::load_from_file("Data/" + basename + ".png");
    // lerr() << "shape " << bitmap.shape();
    assert(bitmap.shape(0) == 32);
    assert(bitmap.shape(1) == 32);

    const Array<float> image = bitmap.to_float_grayscale();
    Array<FixedArray<float, 2>> feature_points = Array<float>::from_dynamic<2>(find_saddle_points(image, -0.01f));
    highlight_features(feature_points, bitmap, 0);

    bitmap.save_to_file("TestOut/" + basename + "-saddle_points.png");
}

int main(int argc, char **argv) {
    test_saddle_detector("sphere_symm");
    test_saddle_detector("sphere_asymm");
    return 0;
}
