#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <cmath>

void stb_alpha_fac(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    float alpha_fac)
{
    if (nrChannels != 4) {
        THROW_OR_ABORT("nrChannels is not 4");
    }
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            int i = (r * width + c) * nrChannels + nrChannels - 1;
            data[i] = (unsigned char)std::round(std::clamp(float(data[i]) * alpha_fac, 0.f, 255.f));
        }
    }
}
