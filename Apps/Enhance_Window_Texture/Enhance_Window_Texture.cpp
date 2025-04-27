#include <Mlib/Arg_Parser.hpp>
#include <cmath>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

float soft_light(float a, float b) {
    return ((1 - 2 * b) * a + 2 * b) * a;
}

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: enhance_window_texture --color color --glass_mask glass_mask --out <out>",
        {},
        {"--color", "--glass_mask", "--out"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unnamed(0);
    StbInfo color = stb_load8(args.named_value("--color"), FlipMode::NONE);
    StbInfo glass_mask = stb_load8(args.named_value("--glass_mask"), FlipMode::NONE);
    if (color.width != glass_mask.width ||
        color.height != glass_mask.height)
    {
        throw std::runtime_error("Differing image sizes");
    }
    if (color.nrChannels != 1 && color.nrChannels != 3) {
        throw std::runtime_error("Color does not have 3 channels (" + std::to_string(color.nrChannels) + ')');
    }
    if (glass_mask.nrChannels != 1) {
        throw std::runtime_error("Glass mask does not have 1 channel (" + std::to_string(glass_mask.nrChannels) + ')');
    }
    auto out = StbInfo<uint8_t>(color.width, color.height, color.nrChannels);
    for (size_t r = 0; r < (size_t)color.height; ++r) {
        for (size_t c = 0; c < (size_t)color.width; ++c) {
            size_t i = r * (size_t)color.width  + c;
            for (size_t d = 0; d < (size_t)color.nrChannels; ++d) {
                float ramp = std::abs((float)r - (float)color.height / 2.f);
                ramp /= (float)color.height;
                // ramp = std::clamp(2.f * ramp, 0.f, 1.f);
                ramp = 1 - ramp;
                out[i * (size_t)color.nrChannels + d] = (uint8_t)std::round(255.f * std::clamp(
                    soft_light(
                        color[i * (size_t)color.nrChannels + d] / 255.f,
                        glass_mask[i] / 255.f * ramp),
                        0.f,
                        1.f));
            }
        }
    }
    if (!stbi_write_png(
        args.named_value("--out").c_str(),
        out.width,
        out.height,
        out.nrChannels,
        out.data(),
        0))
    {
        throw std::runtime_error("Could not write " + args.named_value("--out"));
    }
    return 0;
}
