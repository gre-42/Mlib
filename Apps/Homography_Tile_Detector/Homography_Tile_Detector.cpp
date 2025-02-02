#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/Optical_Flow.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>
#include <Mlib/Sfm/Homography/Homography_From_Points.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

static const size_t window_size = 30;
static const float max_displacement = 16;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: ground_plane_detector image0.png image1.png",
        {},
        {});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unnamed(2);
    const auto raw0 = StbImage3::load_from_file(args.unnamed_value(0));
    const auto raw1 = StbImage3::load_from_file(args.unnamed_value(1));
    const Array<float> image0 = raw0.to_float_grayscale();
    const Array<float> image1 = raw1.to_float_grayscale();
    Array<float> flow;
    Array<bool> global_mask;
    optical_flow(image0, image1, nullptr, ArrayShape{window_size, window_size}, max_displacement, flow, global_mask);
    StbImage3::from_float_grayscale(normalized_and_clipped(flow[0], -max_displacement, max_displacement)).save_to_file("flow-0.png");
    StbImage3::from_float_grayscale(normalized_and_clipped(flow[1], -max_displacement, max_displacement)).save_to_file("flow-1.png");
    // global_mask.row_range(0, global_mask.shape(0) / 2) = false;
    for (size_t tile_id = 0; tile_id < 5; ++tile_id) {
        Array<bool> mask{global_mask.copy()};
        for (size_t it = 0; it < 3; ++it) {
            // H: x -> p, p = x-prime = x'
            Array<float> x{ArrayShape{count_nonzero(mask), 2}};
            Array<float> p{ArrayShape{count_nonzero(mask), 2}};
            size_t i = 0;
            for (size_t r = 0; r < mask.shape(0); ++r) {
                for (size_t c = 0; c < mask.shape(1); ++c) {
                    if (mask(r, c)) {
                        x(i, id1) = i2a(r);
                        x(i, id0) = i2a(c);
                        p(i, id1) = i2a(r) + flow(id1, r, c);
                        p(i, id0) = i2a(c) + flow(id0, r, c);
                        ++i;
                    }
                }
            }
            if (any(Mlib::isnan(x)) || any(Mlib::isnan(p))) {
                throw std::runtime_error("Internal error, NAN");
            }
            // H: x -> p, p = x-prime = x'
            lerr() << "H";
            FixedArray<float, 3, 3> H = homography_from_points(Array<float>::from_dynamic<2>(x), Array<float>::from_dynamic<2>(p));
            H /= H(2, 2);
            std::list<FixedArray<float, 2>> feature_list;
            std::list<FixedArray<float, 2>> Hp_list;
            for (size_t r = 0; r < mask.shape(0); ++r) {
                for (size_t c = 0; c < mask.shape(1); ++c) {
                    FixedArray<float, 2> a{i2a(FixedArray<size_t, 2>{r, c})};
                    FixedArray<float, 2> b{apply_homography(H, a).template row_range<0, 2>()};
                    FixedArray<size_t, 2> ia = a2i(a);
                    FixedArray<size_t, 2> ib = a2i(b);
                    if (all(ia < FixedArray<size_t, 2>{mask.shape(0), mask.shape(1)}) &&
                        all(ib < FixedArray<size_t, 2>{mask.shape(0), mask.shape(1)}) &&
                        mask(ia(0), ia(1)))
                    {
                        if (std::abs(image0(ia(0), ia(1)) - image1(ib(0), ib(1))) < 0.005) {
                            if (r % 10 == 0 && c % 10 == 0) {
                                feature_list.push_back(a);
                                Hp_list.push_back(b);
                            }
                        } else {
                            mask(ia(0), ia(1)) = false;
                        }
                    }
                }
            }
            Array<FixedArray<float, 2>> features{ feature_list };
            Array<FixedArray<float, 2>> Hp{ Hp_list };
            StbImage3 bmpf0{ StbImage3::from_float_grayscale(image0) };
            StbImage3 bmpf1{ StbImage3::from_float_grayscale(image1) };
            lerr() << H;
            // lerr() << Hp;
            highlight_features(features, bmpf0, 1, Rgb24::red());
            highlight_features(Hp, bmpf1, 1, Rgb24::red());
            const std::string suffix = std::to_string(tile_id) + "-" + std::to_string(it) + ".png";
            bmpf0.save_to_file("bmpf0-" + suffix);
            bmpf1.save_to_file("bmpf1-" + suffix);
            StbImage3::from_float_grayscale(mask.casted<float>()).save_to_file("mask-" + suffix);
        }
        global_mask &= !mask;
    }
    return 0;
}
