#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

/**
 *  From: https://stackoverflow.com/a/56370320/2292832
 */
static void lighten(
    unsigned char& data,
    short color)
{
    if (color < 0) {
        data = std::clamp((data * (255 + color)) / 255, 0, 255);
    } else if (color > 0) {
        data = std::clamp(data + ((255 - data) * color) / 255, 0, 255);
    }
}

void stb_lighten(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    short color[3])
{
    if (nrChannels != 3 && nrChannels != 4) {
        THROW_OR_ABORT("nrChannels is not 3 or 4");
    }
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            for (int d = 0; d < 3; ++d) {
                int i = (r * width + c) * nrChannels + d;
                lighten(data[i], color[d]);
            }
        }
    }
}

void stb_lighten_vertical_gradient(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    short color_top[3],
    short color_bottom[3])
{
    if (nrChannels != 3 && nrChannels != 4) {
        THROW_OR_ABORT("nrChannels is not 3 or 4");
    }
    for (int r = 0; r < height; ++r) {
        float t = float(r) / height;
        for (int c = 0; c < width; ++c) {
            for (int d = 0; d < 3; ++d) {
                int i = (r * width + c) * nrChannels + d;
                lighten(data[i], (short)std::round(t * color_top[d] + (1 - t) * color_bottom[d]));
            }
        }
    }
}