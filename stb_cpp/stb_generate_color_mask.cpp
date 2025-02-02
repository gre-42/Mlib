#include <algorithm>
#include <cmath>

void stb_generate_color_mask(
    const unsigned char* in_data,
    unsigned char* out_data,
    int width,
    int height,
    int nrChannels,
    short* color,
    unsigned short near,
    unsigned short far)
{
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            int distance = 0;
            for (int d = 0; d < nrChannels; ++d) {
                int i_in = (r * width + c) * nrChannels + d;
                distance += std::abs((short)in_data[i_in] - color[d]);
            }
            int i_out = r * width + c;
            if (distance <= near) {
                out_data[i_out] = 255;
            } else if (distance >= far) {
                out_data[i_out] = 0;
            } else {
                out_data[i_out] = (255 * (far - distance)) / (far - near);
            }
        }
    }
}
