#include <algorithm>
#include <iostream>
#include <stdexcept>

/**
 *  From: https://stackoverflow.com/a/56370320/2292832
 */
void stb_lighten(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    short color[3])
{
    if (nrChannels != 3 && nrChannels != 4) {
        throw std::runtime_error("nrChannels is not 3 or 4");
    }
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            for (int d = 0; d < 3; ++d) {
                int i = (r * width + c) * nrChannels + d;
                if (color[d] < 0) {
                    data[i] = std::clamp((data[i] * (255 + color[d])) / 255, 0, 255);
                } else if (color[d] > 0) {
                    data[i] = std::clamp(data[i] + ((255 - data[i]) * color[d]) / 255, 0, 255);
                }
            }
        }
    }
}
