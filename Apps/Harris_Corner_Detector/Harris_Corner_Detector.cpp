#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/Resample/Pyramid.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Sfm/Disparity/Corresponding_Features_In_Candidate_List.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Stats/Variance.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

int main(int argc, char** argv) {
    enable_floating_point_exceptions();
    const ArgParser parser(
        "Usage: harris_corner_detector "
        "source "
        "destination "
        "ncorners "
        "[--source1 <source1>] "
        "[--k <k>] "
        "[--distance-sigma <distance-sigma>] "
        "[--multi-scale] "
        "[--size <marker-size>] "
        "[--response <response>] "
        "[--clip-min <clip-min>] "
        "[--clip-max <clip-max>]",
        { "--multi-scale" },
        { "--source1",
          "--k",
          "--distance-sigma",
          "--size",
          "--response",
          "--clip-min",
          "--clip-max" });
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed(3);
        auto bitmap0 = StbImage3::load_from_file(args.unnamed_value(0));
        Array<float> response0 = harris_response(
            bitmap0.to_float_grayscale(),
            safe_stof(args.named_value("--k", "0.05")));
        Array<bool> feature_mask0;
        if (args.has_named("--multi-scale")) {
            feature_mask0 = multi_scale_harris(bitmap0.to_float_grayscale(), 4);
        } else {
            feature_mask0 = ones<bool>(bitmap0.shape());
        }
        Array<FixedArray<float, 2>> corners0 = Array<float>::from_dynamic<2>(find_nfeatures(
            response0,
            feature_mask0,
            safe_stoi(args.unnamed_value(2)),
            safe_stof(args.named_value("--distance-sigma", "0"))));
        // std::cout << "Found " << corners.shape(0) << " corners." << std::endl;
        {
            StbImage3 bmp{bitmap0.copy()};
            highlight_features(
                corners0,
                bmp,
                safe_stoi(args.named_value("--size", "1")));
            bmp.save_to_file(args.unnamed_value(1));
        }
        lerr() << "V[response]=" << nanvar(response0);
        lerr() << "E[response]=" << nanmean(response0);
        lerr() << "min[response]=" << nanmin(response0);
        lerr() << "max[response]=" << nanmax(response0);
        if (args.has_named_value("--response")) {
            //PpmImage::from_float_grayscale(normalized_and_clipped(response, -float(5e-5), float(5e-5)))
            StbImage3::from_float_grayscale(
                normalized_and_clipped(
                    response0,
                    safe_stof(args.named_value("--clip-min", "-0.002")),
                    safe_stof(args.named_value("--clip-max", "0.002"))))
                .save_to_file(args.named_value("--response"));
        }
        if (args.has_named_value("--source1")) {
            auto bitmap1 = StbImage3::load_from_file(args.named_value("--source1"));
            if (any(bitmap0.shape() != bitmap1.shape())) {
                throw std::runtime_error("Images have different shapes");
            }
            Array<float> response1 = harris_response(
                bitmap1.to_float_grayscale(),
                safe_stof(args.named_value("--k", "0.05")));
            Array<bool> feature_mask1;
            if (args.has_named("--multi-scale")) {
                feature_mask1 = multi_scale_harris(bitmap1.to_float_grayscale(), 4);
            } else {
                feature_mask1 = ones<bool>(bitmap1.shape());
            }
            Array<FixedArray<float, 2>> corners1 = Array<float>::from_dynamic<2>(find_nfeatures(
                response1,
                feature_mask1,
                safe_stoi(args.unnamed_value(2)),
                safe_stof(args.named_value("--distance-sigma", "0"))));
            CorrespondingFeaturesInCandidateList cf{corners0, corners1, bitmap0.to_float_rgb(), bitmap1.to_float_rgb(), 10};
            {
                StbImage3 bmp{ bitmap0.copy() };
                highlight_features(cf.y0, bmp, 2, Rgb24::red());
                highlight_features(cf.y1, bmp, 2, Rgb24::blue());
                highlight_feature_correspondences(cf.y0, cf.y1, bmp, 0, Rgb24::red(), rvalue_address(Rgb24::nan()));
                bmp.save_to_file("features10_0.png");
            }
            {
                StbImage3 bmp{ bitmap1.copy() };
                highlight_features(cf.y0, bmp, 2, Rgb24::red());
                highlight_features(cf.y1, bmp, 2, Rgb24::blue());
                highlight_feature_correspondences(cf.y0, cf.y1, bmp, 0, Rgb24::red(), rvalue_address(Rgb24::nan()));
                bmp.save_to_file("features10_1.png");
            }
        }
    } catch (const std::runtime_error& e) {
        lerr() << "ERROR: " << e.what();
        return 1;
    }
    return 0;
}
