#include "stb_set_alpha.hpp"

void stb_set_alpha(
    const unsigned char* rgb,
    const unsigned char* alpha,
    unsigned char* rgba,
    int width,
    int height,
    int alpha_width,
    int alpha_height)
{
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            for (int d = 0; d < 3; ++d) {
                int i3 = (r * width + c) * 3 + d;
                int i4 = (r * width + c) * 4 + d;
                rgba[i4] = rgb[i3];
            }
            {
                int i1 = ((r % alpha_height) * alpha_width + (c % alpha_width)) * 1;
                int i4 = (r * width + c) * 4 + 3;
                rgba[i4] = alpha[i1];
            }
        }
    }
}
