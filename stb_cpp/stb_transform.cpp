#include <algorithm>
#include <cmath>

void stb_transform(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    float times,
    float plus)
{
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            for (int d = 0; d < nrChannels; ++d) {
                int i = (r * width + c) * nrChannels + d;
                data[i] = std::clamp(
                    (unsigned char)std::round(data[i] * times + plus * 255.f),
                    (unsigned char)0,
                    (unsigned char)255);
            }
        }
    }
}
