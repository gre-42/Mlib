#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Images/Resample/Pyramid.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Stats/Variance.hpp>
#include <Mlib/String.hpp>

using namespace Mlib;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: harris_corner_detector source destination ncorners [--size <marker-size>] [--distance-sigma <distance-sigma>] [--multi-scale] [--response <response>] [--feature-mask <feature-mask>]",
        {"--multi-scale"},
        {"--size", "--distance-sigma", "--response", "--feature-mask"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unamed(3);
    auto bitmap = PpmImage::load_from_file(args.unnamed_value(0));
    ArrayShape smooth_width(2);
    Array<bool> feature_mask;
    Array<float> response = harris_response(bitmap.to_float_grayscale(), &feature_mask);
    if (args.has_named("--multi-scale")) {
        feature_mask = multi_scale_harris(bitmap.to_float_grayscale(), 4);
    }
    if (args.has_named_value("--feature-mask")) {
        PpmImage::from_float_grayscale(feature_mask.casted<float>())
        .save_to_file(args.named_value("--feature-mask"));
    } else {
        feature_mask = ones<bool>(bitmap.shape());
    }
    Array<float> corners = find_nfeatures(
        -response,
        feature_mask,
        safe_stoi(args.unnamed_value(2)),
        safe_stod(args.named_value("--distance-sigma", "0")));
    // std::cout << "Found " << corners.shape(0) << " corners." << std::endl;
    highlight_features(
        corners,
        bitmap,
        safe_stoi(args.named_value("--size", "1")));
    bitmap.save_to_file(args.unnamed_value(1));
    std::cerr << "V[response]=" << nanvar(response) << std::endl;
    std::cerr << "E[response]=" << nanmean(response) << std::endl;
    std::cerr << "min[response]=" << nanmin(response) << std::endl;
    std::cerr << "max[response]=" << nanmax(response) << std::endl;
    if (args.has_named_value("--response")) {
        //PpmImage::from_float_grayscale(normalized_and_clipped(response, -float(5e-5), float(5e-5)))
        PpmImage::from_float_grayscale(normalized_and_clipped(-response, 0.f, 0.002f))
        .save_to_file(args.named_value("--response"));
    }
    return 0;
}
