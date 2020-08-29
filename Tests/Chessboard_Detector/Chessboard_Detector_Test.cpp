#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Sfm/Components/Detect_Chessboard.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>
#include <iostream>

using namespace Mlib;
using namespace Mlib::Sfm;

void test_chessboard_detector() {
    const auto bitmap = PpmImage::load_from_file("Data/chessboard1.ppm");

    const Array<float> image = bitmap.to_float_grayscale();
    PpmImage bmp;
    Array<float> x;
    Array<float> y;
    detect_chessboard(image, ArrayShape{6, 9}, x, y, bmp);

    assert_true(all(x.shape() == ArrayShape{6 * 9, 4}));
    assert_true(all(y.shape() == ArrayShape{6 * 9, 3}));

    // std::list<Array<float>> feature_points = find_saddle_points(image);
    // highlight_features(feature_points, res);

    bmp.save_to_file("TestOut/chessboard1_detected.ppm");
}

void test_inverse_homography() {
    Array<float> homography{
        {0.5, 0.6, 0.2},
        {0.1, 0.7, 0.15},
        {0.25, 0.9, 2}};
    Array<float> a{0.31, 0.45, 1};
    Array<float> b = apply_homography(homography, a);
    Array<float> bi = apply_inverse_homography(homography, b);
    assert_allclose(bi, a);
}

int main(int argc, char **argv) {
    test_inverse_homography();
    test_chessboard_detector();
    return 0;
}
