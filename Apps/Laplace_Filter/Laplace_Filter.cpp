#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geography/Heightmaps/Load_Heightmap_From_File.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <iostream>

using namespace Mlib;

void laplace_filter_file(
    const std::string& source,
    const std::string& destination,
    float sigma,
    bool is_heightmap)
{
    if (is_heightmap) {
        auto bitmap = load_heightmap_from_file<double>(source);
        StbImage1 dest = StbImage1::from_float_grayscale(normalized_and_clipped(gaussian_filter_NWE<double>(laplace_filter<double>(bitmap, NAN), sigma, NAN)).casted<float>());
        dest.save_to_file(destination);
    } else {
        auto bitmap = StbImage3::load_from_file(source).to_float_rgb();
        StbImage3 dest;
        if (true) {
            dest = StbImage3::from_float_rgb(normalized_and_clipped(multichannel_gaussian_filter_NWE(multichannel_laplace_filter(bitmap, NAN), sigma, NAN)));
        } else {
            dest = StbImage3::from_float_rgb(normalized_and_clipped(multichannel_gaussian_filter_NWE(multichannel_central_sad_filter(bitmap), sigma, NAN)));
        }
        dest.save_to_file(destination);
    }
}

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: laplace_filter source destination --sigma <sigma> [--is_heightmap]",
        {"--is_heightmap"},
        {"--sigma"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed(2);
        laplace_filter_file(
            args.unnamed_value(0),
            args.unnamed_value(1),
            safe_stof(args.named_value("--sigma")),
            args.has_named("--is_heightmap"));
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
