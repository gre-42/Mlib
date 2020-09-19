#include <algorithm>
#include <iostream>
#include <stdexcept>

bool stb_match_color_rgb(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    unsigned char color[3])
{
    if (nrChannels != 3 && nrChannels != 4) {
        throw std::runtime_error("nrChannels is not 3 or 4");
    }
    float means[3] = {};
    if (nrChannels == 3) {
        for(int r = 0; r < height; ++r) {
            for(int c = 0; c < width; ++c) {
                for(int d = 0; d < 3; ++d) {
                    means[d] += data[(r * width + c) * nrChannels + d];
                }
            }
        }
        for(int d = 0; d < 3; ++d) {
            means[d] /= width * height * nrChannels;
        }
    } else {
        float sum_alpha = 0;
        for(int r = 0; r < height; ++r) {
            for(int c = 0; c < width; ++c) {
                int i = (r * width + c) * nrChannels;
                float alpha = float(data[i + 3]) / 255;
                for(int d = 0; d < 3; ++d) {
                    means[d] += data[i + d] * alpha;
                }
                sum_alpha += alpha;
            }
        }
        if (sum_alpha == 0) {
            return false;
        }
        for(int d = 0; d < 3; ++d) {
            means[d] /= sum_alpha;
        }
    }
    for(int r = 0; r < height; ++r) {
        for(int c = 0; c < width; ++c) {
            for(int d = 0; d < 3; ++d) {
                int i = (r * width + c) * nrChannels + d;
                data[i] = std::clamp(data[i] - means[d] + color[d], 0.f, 255.f);
            }
        }
    }
    return true;
}
