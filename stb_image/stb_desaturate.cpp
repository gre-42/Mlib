#include <algorithm>
#include <iostream>
#include <stdexcept>

void stb_desaturate(
    unsigned char* data,
    int width,
    int height,
    int nrChannels)
{
    if (nrChannels != 3 && nrChannels != 4) {
        throw std::runtime_error("nrChannels is not 3 or 4");
    }
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            unsigned int mean = 0;
            for (int d = 0; d < 3; ++d) {
                int i = (r * width + c) * nrChannels + d;
                mean += data[i];
            }
            mean = std::clamp(mean / 3, 0u, 255u);
            for (int d = 0; d < 3; ++d) {
                int i = (r * width + c) * nrChannels + d;
                data[i] = mean;
            }
        }
    }
}
