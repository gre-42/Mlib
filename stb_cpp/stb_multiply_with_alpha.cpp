#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>
#include <stb_cpp/stb_image_load.hpp>

StbInfo<unsigned char> stb_multiply_with_alpha(
    unsigned char* data,
    int width,
    int height,
    int nrChannels)
{
    if (nrChannels != 4) {
        THROW_OR_ABORT("nrChannels is not 4");
    }
    auto result = StbInfo<unsigned char>(width, height, nrChannels - 1);
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            int i = (r * width + c) * nrChannels;
            auto fac = float(data[i + nrChannels - 1]) / 255.f;
            for (int j = 0; j < nrChannels; ++j) {
                result(c, r, j) = (unsigned char)std::round(data[i + j] * fac);
            }
        }
    }
    return result;
}
