#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Box_Filter.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <iostream>

using namespace Mlib;

void highlight_saddle_points(
    const std::string& source,
    const std::string& destination,
    size_t width,
    size_t niter,
    size_t marker_size)
{
    auto bitmap = StbImage3::load_from_file(source);

    Array<float> image = bitmap.to_float_grayscale();
    for (size_t i = 0; i < niter; ++i) {
        image = box_filter_NWE(image, ArrayShape{ width, width });
    }
    Array<FixedArray<float, 2>> feature_points = Array<float>::from_dynamic<2>(find_saddle_points(image));
    highlight_features(feature_points, bitmap, marker_size);

    bitmap.save_to_file(destination);
}

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: highlight_saddle_points source destination [--width <width> (=5)] [--niter <niter> (=0)] [--size <marker-size>]",
        {},
        {"--width", "--niter", "--size"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unnamed(2);
    highlight_saddle_points(
        args.unnamed_value(0),
        args.unnamed_value(1),
        safe_stoz(args.named_value("--width", "5")),
        safe_stoz(args.named_value("--niter", "0")),
        safe_stoz(args.named_value("--size", "1")));
    return 0;
}
