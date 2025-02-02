#include "stb_blend.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <cmath>

void stb_average(
    const unsigned char* src0,
    const unsigned char* src1,
    unsigned char* dest,
    int width,
    int height,
    int width1,
    int height1,
    int nrChannels0,
    int nrChannels1,
    int nrChannelsDest)
{
    if (nrChannels0 != 3) {
        THROW_OR_ABORT("nrChannels0 is not 3");
    }
    if (nrChannels1 != 3) {
        THROW_OR_ABORT("nrChannels1 is not 3");
    }
    if (nrChannelsDest != 3) {
        THROW_OR_ABORT("nrChannels0 differst from nrChannelsDest");
    }
    stb_blend(
        src0,
        src1,
        dest,
        width,
        height,
        width1,
        height1,
        nrChannels0,
        nrChannels1,
        nrChannelsDest,
        [](const unsigned char* s0, const unsigned char* s1, unsigned char* d)
        {
            for (size_t i = 0; i < 3; ++i) {
                d[i] = (unsigned char)std::clamp(std::round(((float)s0[i] + (float)s1[i]) / 2.f), 0.f, 255.f);
            }
        });
}

void stb_multiply_color(
    const unsigned char* src0,
    const unsigned char* src1,
    unsigned char* dest,
    int width,
    int height,
    int width1,
    int height1,
    int nrChannels0,
    int nrChannels1,
    int nrChannelsDest)
{
    if ((nrChannels0 != 3) && (nrChannels0 != 4)) {
        THROW_OR_ABORT("nrChannels0 is not 3 or 4");
    }
    if (nrChannels1 != 3) {
        THROW_OR_ABORT("nrChannels1 is not 3");
    }
    if (nrChannels0 != nrChannelsDest) {
        THROW_OR_ABORT("nrChannels0 differst from nrChannelsDest");
    }
    stb_blend(
        src0,
        src1,
        dest,
        width,
        height,
        width1,
        height1,
        nrChannels0,
        nrChannels1,
        nrChannelsDest,
        [dest, src0, nrChannels0](const unsigned char* s0, const unsigned char* s1, unsigned char* d)
        {
            for (size_t i = 0; i < 3; ++i) {
                d[i] = (unsigned char)std::clamp(std::round(((float)s0[i] * (float)s1[i]) / 255.f), 0.f, 255.f);
            }
            if ((nrChannels0 == 4) && (dest != src0)) {
                d[3] = s0[3];
            }
        });
}

void stb_alpha_blend(
    const unsigned char* src0,
    const unsigned char* src1,
    unsigned char* dest,
    int width,
    int height,
    int width1,
    int height1,
    int nrChannels0,
    int nrChannels1,
    int nrChannelsDest)
{
    if ((nrChannels0 != 3) && (nrChannels0 != 4)) {
        THROW_OR_ABORT("nrChannels0 is not 3 or 4");
    }
    if (nrChannels1 != 4) {
        THROW_OR_ABORT("nrChannels1 is not 4");
    }
    if (nrChannels0 != nrChannelsDest) {
        THROW_OR_ABORT("nrChannels0 differst from nrChannelsDest");
    }
    stb_blend(
        src0,
        src1,
        dest,
        width,
        height,
        width1,
        height1,
        nrChannels0,
        nrChannels1,
        nrChannelsDest,
        [dest, src0, nrChannels0](const unsigned char* s0, const unsigned char* s1, unsigned char* d)
        {
            float w1 = s1[3] / 255.f;
            float w0 = 1 - w1;
            for (size_t i = 0; i < 3; ++i) {
                d[i] = (unsigned char)std::clamp(std::round((float)s0[i] * w0 + (float)s1[i] * w1), 0.f, 255.f);
            }
            if ((nrChannels0 == 4) && (dest != src0)) {
                d[3] = s0[3];
            }
        });
}
