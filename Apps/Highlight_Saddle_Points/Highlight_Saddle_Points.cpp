#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/PpmImage.hpp>
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
    auto bitmap = PpmImage::load_from_file(source);

    Array<float> image = bitmap.to_float_grayscale();
    for (size_t i = 0; i < niter; ++i) {
        image = box_filter(image, ArrayShape{width, width}, NAN);
    }
    Array<float> feature_points = find_saddle_points(image);
    highlight_features(feature_points, bitmap, marker_size);

    bitmap.save_to_file(destination);
}

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: highlight_saddle_points source destination [--width <width> (=5)] [--niter <niter> (=0)] [--size <marker-size>]",
        {},
        {"--width", "--niter", "--size"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unamed(2);
    highlight_saddle_points(
        args.unnamed_value(0),
        args.unnamed_value(1),
        safe_stoi(args.named_value("--width", "5")),
        safe_stoi(args.named_value("--niter", "0")),
        safe_stoi(args.named_value("--size", "1")));
    return 0;
}
