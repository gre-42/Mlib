#pragma once

template <class TOperation>
void stb_blend(
    const unsigned char* src0,
    const unsigned char* src1,
    unsigned char* dest,
    int width,
    int height,
    int width1,
    int height1,
    int nrChannels0,
    int nrChannels1,
    int nrChannelsDest,
    const TOperation& blend)
{
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            int i0 = (r * width + c) * nrChannels0;
            int i1 = ((r % height1) * width1 + (c % width1)) * nrChannels1;
            int id = (r * width + c) * nrChannelsDest;
            blend(src0 + i0, src1 + i1, dest + id);
        }
    }
}

void stb_average(
    const unsigned char* data0,
    const unsigned char* data1,
    unsigned char* dest,
    int width,
    int height,
    int width1,
    int height1,
    int nrChannels0,
    int nrChannels1,
    int nrChannelsDest);

void stb_multiply_color(
    const unsigned char* data0,
    const unsigned char* data1,
    unsigned char* dest,
    int width,
    int height,
    int width1,
    int height1,
    int nrChannels0,
    int nrChannels1,
    int nrChannelsDest);

void stb_alpha_blend(
    const unsigned char* data0,
    const unsigned char* data1,
    unsigned char* dest,
    int width,
    int height,
    int width1,
    int height1,
    int nrChannels0,
    int nrChannels1,
    int nrChannelsDest);
