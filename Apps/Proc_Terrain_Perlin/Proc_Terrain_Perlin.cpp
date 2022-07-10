#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <siv/PerlinNoise.hpp>
#include <stb_image/stb_image_write.h>

using namespace Mlib;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: proc_terrain_perlin"
        " --out <out.pgm>"
        " --size <size>"
        " --frequency <frequency>"
        " --octaves <octaves>"
        " --seed <seed>"
        " [--sigma <sigma>]"
        " [--alpha <alpha>]"
        " [--min <min>]"
        " [--max <max>]"
        " [--invert]",
        {"--invert"},
        {"--out",
         "--size",
         "--frequency",
         "--octaves",
         "--seed",
         "--sigma",
         "--alpha",
         "--min",
         "--max"});

    try {
        const auto args = parser.parsed(argc, argv);

        float frequency = safe_stof(args.named_value("--frequency"));
        int octaves = safe_stoi(args.named_value("--octaves"));
        unsigned int seed = safe_stou(args.named_value("--seed"));

        Array<float> out{ArrayShape{ safe_stoz(args.named_value("--size")), safe_stoz(args.named_value("--size")) }};

        const siv::PerlinNoise perlin(seed);
        const float fx = out.shape(1) / frequency;
        const float fy = out.shape(0) / frequency;

        for (size_t y = 0; y < out.shape(0); ++y)
        {
            for (size_t x = 0; x < out.shape(1); ++x)
            {
                out(y, x) = (float)perlin.accumulatedOctaveNoise2D_0_1(x / fx, y / fy, octaves);
            }
        }
        if (float sigma = safe_stof(args.named_value("--sigma", "0")); sigma != 0) {
            out = gaussian_filter_NWE(out, sigma, float{NAN});
        }
        float dmin = safe_stof(args.named_value("--min", "0"));
        float dmax = safe_stof(args.named_value("--max", "1"));
        if (float alpha = safe_stof(args.named_value("--alpha", "1")); alpha != 1) {
            out = out.applied([alpha](float v){return std::pow(v, alpha);});
            out = normalized_and_clipped(out, std::pow(dmin, alpha), std::pow(dmax, alpha));
        } else if ((dmin != 0.f) || (dmax != 1.f)) {
            out = normalized_and_clipped(out, dmin, dmax);
        }
        if (args.has_named("--invert")) {
            out = 1.f - out;
        }
        PgmImage::from_float(out).save_to_file(args.named_value("--out"));
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
