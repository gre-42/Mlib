#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/Optical_Flow.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>
#include <Mlib/Sfm/Homography/Homography_From_Points.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

static const size_t window_size = 30;
static const float max_displacement = 16;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: ground_plane_detector image0.ppm image1.ppm",
        {},
        {});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unamed(2);
    const auto raw0 = PpmImage::load_from_file(args.unnamed_value(0));
    const auto raw1 = PpmImage::load_from_file(args.unnamed_value(1));
    const Array<float> image0 = raw0.to_float_grayscale();
    const Array<float> image1 = raw1.to_float_grayscale();
    Array<float> flow;
    Array<bool> global_mask;
    optical_flow(image0, image1, nullptr, ArrayShape{window_size, window_size}, max_displacement, flow, global_mask);
    PpmImage::from_float_grayscale(normalized_and_clipped(flow[0], -max_displacement, max_displacement)).save_to_file("flow-0.ppm");
    PpmImage::from_float_grayscale(normalized_and_clipped(flow[1], -max_displacement, max_displacement)).save_to_file("flow-1.ppm");
    // global_mask.row_range(0, global_mask.shape(0) / 2) = false;
    for (size_t tile_id = 0; tile_id < 5; ++tile_id) {
        Array<bool> mask{global_mask.copy()};
        for (size_t it = 0; it < 3; ++it) {
            // H: x -> p, p = x-prime = x'
            Array<float> x{ArrayShape{count_nonzero(mask), 3}};
            Array<float> p{ArrayShape{count_nonzero(mask), 3}};
            size_t i = 0;
            for (size_t r = 0; r < mask.shape(0); ++r) {
                for (size_t c = 0; c < mask.shape(1); ++c) {
                    if (mask(r, c)) {
                        x(i, id1) = i2a(r);
                        x(i, id0) = i2a(c);
                        x(i, 2) = 1;
                        p(i, id1) = i2a(r) + flow(id1, r, c);
                        p(i, id0) = i2a(c) + flow(id0, r, c);
                        p(i, 2) = 1;
                        ++i;
                    }
                }
            }
            if (any(isnan(x)) || any(isnan(p))) {
                throw std::runtime_error("Internal error, NAN");
            }
            // H: x -> p, p = x-prime = x'
            std::cerr << "H" << std::endl;
            Array<float> H = homography_from_points(x, p);
            H /= H(2, 2);
            std::list<Array<float>> feature_list;
            std::list<Array<float>> Hp_list;
            for (size_t r = 0; r < mask.shape(0); ++r) {
                for (size_t c = 0; c < mask.shape(1); ++c) {
                    Array<float> a{i2a(ArrayShape{r, c})};
                    Array<float> b{apply_homography(H, homogenized_3(a)).row_range(0, 2)};
                    ArrayShape ia = a2i(a);
                    ArrayShape ib = a2i(b);
                    if (all(ia < mask.shape()) &&
                        all(ib < mask.shape()) &&
                        mask(ia))
                    {
                        if (std::abs(image0(ia) - image1(ib)) < 0.005) {
                            if (r % 10 == 0 && c % 10 == 0) {
                                feature_list.push_back(a);
                                Hp_list.push_back(b);
                            }
                        } else {
                            mask(ia) = false;
                        }
                    }
                }
            }
            Array<float> features = Array<float>{feature_list};
            Array<float> Hp{Hp_list};
            PpmImage bmpf0{PpmImage::from_float_grayscale(image0)};
            PpmImage bmpf1{PpmImage::from_float_grayscale(image1)};
            std::cerr << H << std::endl;
            // std::cerr << Hp << std::endl;
            highlight_features(features, bmpf0, 1, Rgb24::red());
            highlight_features(Hp, bmpf1, 1, Rgb24::red());
            const std::string suffix = std::to_string(tile_id) + "-" + std::to_string(it) + ".ppm";
            bmpf0.save_to_file("bmpf0-" + suffix);
            bmpf1.save_to_file("bmpf1-" + suffix);
            PpmImage::from_float_grayscale(mask.casted<float>()).save_to_file("mask-" + suffix);
        }
        global_mask &= !mask;
    }
    return 0;
}
