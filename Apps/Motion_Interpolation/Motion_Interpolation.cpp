#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Optical_Flow.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Images/Registration.hpp>
#include <Mlib/String.hpp>

using namespace Mlib;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: motion_interpolation <sources ...> --out-prefix <out-prefix>",
        {},
        {"--out-prefix"});
    const auto args = parser.parsed(argc, argv);
    std::vector<std::string> sources = args.unnamed_values();
    for (size_t i = 1; i < sources.size(); ++i) {
        auto s0 = PpmImage::load_from_file(sources[i - 1]);
        auto s1 = PpmImage::load_from_file(sources[i]);
        auto g0 = s0.to_float_grayscale();
        auto g1 = s1.to_float_grayscale();
        Array<float> flow;
        Array<bool> mask;
        optical_flow(g0, g1, nullptr, ArrayShape{5, 5}, 300, flow, mask);
        auto in = 0.5f * (apply_displacement(g0, -0.5f * flow) + apply_displacement(g1, 0.5f * flow));
        PpmImage::from_float_grayscale(in).save_to_file(args.named_value("--out-prefix") + std::to_string(i) + ".ppm");
    }
    return 0;
}
