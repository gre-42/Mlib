#include <algorithm>
#include <cmath>
#include <stdexcept>

void stb_alpha_fac(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    float alpha_fac,
    float alpha_exponent)
{
    if (nrChannels != 4) {
        throw std::runtime_error("nrChannels is not 4");
    }
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            int i = (r * width + c) * nrChannels + nrChannels - 1;
            auto v = float(data[i]) / 255.f;
            v = std::pow(v, alpha_exponent) * alpha_fac;
            data[i] = (unsigned char)std::round(255.f * std::clamp(v, 0.f, 1.f));
        }
    }
}
