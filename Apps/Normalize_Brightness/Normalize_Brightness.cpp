#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

void normalize_brightness(
    const std::string& source,
    const std::string& destination,
    float sigma)
{
    auto bitmap = PpmImage::load_from_file(source);
    auto color = bitmap.to_float_rgb();
    auto brightness = gaussian_filter_NWE(bitmap.to_float_grayscale(), sigma, NAN);
    Array<float> result{ArrayShape{color.shape()}};
    for (size_t d = 0; d < 3; ++d) {
        result[d] = color[d] / brightness;
    }
    PpmImage dest = PpmImage::from_float_rgb(normalized_and_clipped(result));
    dest.save_to_file(destination);
}

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: normalize_brightness source destination --sigma <sigma>",
        {},
        {"--sigma"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unamed(2);
    normalize_brightness(
        args.unnamed_value(0),
        args.unnamed_value(1),
        safe_stof(args.named_value("--sigma")));
    return 0;
}
