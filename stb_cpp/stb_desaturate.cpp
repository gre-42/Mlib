#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

void stb_desaturate(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    float amount)
{
    if (nrChannels != 3 && nrChannels != 4) {
        THROW_OR_ABORT("nrChannels is not 3 or 4");
    }
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            auto* dat = &data[(r * width + c) * nrChannels];
            float mean = 0.2989f * dat[0] + 0.5870f * dat[1] + 0.1140f * dat[2];
            for (int d = 0; d < 3; ++d) {
                dat[d] = (int)std::round(std::clamp(amount * mean  + (1 - amount) * dat[d], 0.f, 255.f));
            }
        }
    }
}
