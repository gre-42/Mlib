#include <Mlib/Assert.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Sfm/Components/Detect_Chessboard.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>
#include <iostream>

using namespace Mlib;
using namespace Mlib::Sfm;

void test_chessboard_detector() {
    const auto bitmap = StbImage3::load_from_file("Data/chessboard1.png");

    const Array<float> image = bitmap.to_float_grayscale();
    StbImage3 bmp;
    Array<FixedArray<float, 2>> x;
    Array<FixedArray<float, 2>> y;
    detect_chessboard(image, ArrayShape{6, 9}, x, y, bmp);

    assert_true(all(x.shape() == ArrayShape{6 * 9}));
    assert_true(all(y.shape() == ArrayShape{6 * 9}));

    // std::list<Array<float>> feature_points = find_saddle_points(image);
    // highlight_features(feature_points, res);

    bmp.save_to_file("TestOut/chessboard1_detected.png");
}

void test_inverse_homography() {
    auto homography = FixedArray<float, 3, 3>::init(
        0.5f, 0.6f, 0.2f,
        0.1f, 0.7f, 0.15f,
        0.25f, 0.9f, 2.f);
    FixedArray<float, 2> a{0.31f, 0.45f};
    FixedArray<float, 2> b = apply_homography(homography, a);
    FixedArray<float, 2> bi = apply_inverse_homography(homography, b);
    assert_allclose(bi.to_array(), a.to_array());
}

int main(int argc, char **argv) {
    test_inverse_homography();
    test_chessboard_detector();
    return 0;
}
