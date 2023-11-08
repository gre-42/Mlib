#include <algorithm>
#include <cmath>

void stb_transform(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    float times,
    float plus,
    bool abs)
{
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            for (int d = 0; d < nrChannels; ++d) {
                int i = (r * width + c) * nrChannels + d;
                float v = data[i] * times + plus * 255.f;
                if (abs) {
                    v = std::abs(v);
                }
                data[i] = (unsigned char)std::clamp(
                    std::round(v),
                    0.f,
                    255.f);
            }
        }
    }
}
