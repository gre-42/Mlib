#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <iostream>

using namespace Mlib;

void laplace_filter_file(
    const std::string& source,
    const std::string& destination,
    float sigma)
{
    auto bitmap = PpmImage::load_from_file(source);
    PpmImage dest;
    if (true) {
        dest = PpmImage::from_float_rgb(normalized_and_clipped(multichannel_gaussian_filter_NWE(multichannel_laplace_filter(bitmap.to_float_rgb(), NAN), sigma, NAN)));
    } else {
        dest = PpmImage::from_float_rgb(normalized_and_clipped(multichannel_gaussian_filter_NWE(multichannel_central_sad_filter(bitmap.to_float_rgb()), sigma, NAN)));
    }
    dest.save_to_file(destination);
}

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: laplace_filter source destination --sigma <sigma>",
        {},
        {"--sigma"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unamed(2);
    laplace_filter_file(
        args.unnamed_value(0),
        args.unnamed_value(1),
        safe_stof(args.named_value("--sigma")));
    return 0;
}
