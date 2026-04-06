#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

void stb_desaturate(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    float amount,
    float exponent)
{
    if (nrChannels != 3 && nrChannels != 4) {
        throw std::runtime_error("nrChannels is not 3 or 4");
    }
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            auto* dat = &data[(r * width + c) * nrChannels];
            auto mean = 0.2989f * dat[0] + 0.5870f * dat[1] + 0.1140f * dat[2];
            auto amount2 = (exponent == 0.f)
                ? amount
                : std::pow(mean / 255.f, exponent) * amount;
            for (int d = 0; d < 3; ++d) {
                dat[d] = (unsigned char)std::round(std::clamp(std::lerp((float)dat[d], mean, amount2), 0.f, 255.f));
            }
        }
    }
}
