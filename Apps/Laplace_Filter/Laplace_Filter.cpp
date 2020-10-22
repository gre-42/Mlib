#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/String.hpp>
#include <iostream>

using namespace Mlib;

void laplace_filter_file(
    const std::string& source,
    const std::string& destination,
    float sigma)
{
    auto bitmap = PpmImage::load_from_file(source);
    auto dest = PpmImage::from_float_rgb(normalized_and_clipped(multichannel_gaussian_filter_NWE(multichannel_laplace_filter(bitmap.to_float_rgb(), NAN), sigma, NAN)));
    dest.save_to_file(destination);
}

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: box_filter source destination --sigma <sigma>",
        {},
        {"--sigma"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unamed(2);
    laplace_filter_file(
        args.unnamed_value(0),
        args.unnamed_value(1),
        safe_stoi(args.named_value("--sigma")));
    return 0;
}
