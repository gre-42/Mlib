#include <algorithm>
#include <iostream>
#include <stdexcept>

void stb_replace_color(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    unsigned char color_to_replace[3],
    unsigned char replacement_color[3],
    unsigned char tolerance)
{
    if (nrChannels != 3 && nrChannels != 4) {
        throw std::runtime_error("nrChannels is not 3 or 4");
    }
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            unsigned char* pixel = data + (r * width + c) * nrChannels;
            for (int d = 0; d < 3; ++d) {
                if (std::abs((short)pixel[d] - (short)color_to_replace[d]) > tolerance) {
                    goto skip;
                }
            }
            for (int d = 0; d < 3; ++d) {
                pixel[d] = replacement_color[d];
            }
            skip:;
        }
    }
}
