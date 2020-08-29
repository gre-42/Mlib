#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <iostream>

using namespace Mlib;

void test_saddle_detector(const std::string& basename) {
    auto bitmap = PpmImage::load_from_file("Data/" + basename + ".ppm");
    // std::cerr << "shape " << bitmap.shape() << std::endl;
    assert(bitmap.shape(0) == 32);
    assert(bitmap.shape(1) == 32);

    const Array<float> image = bitmap.to_float_grayscale();
    Array<float> feature_points = find_saddle_points(image, -0.01);
    highlight_features(feature_points, bitmap, 0);

    bitmap.save_to_file("TestOut/" + basename + "-saddle_points.ppm");
}

int main(int argc, char **argv) {
    test_saddle_detector("sphere_symm");
    test_saddle_detector("sphere_asymm");
    return 0;
}
