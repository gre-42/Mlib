#pragma once

void stb_generate_color_mask(
    const unsigned char* in_data,
    unsigned char* out_data,
    int width,
    int height,
    int nrChannels,
    short* color,
    unsigned short near,
    unsigned short far);

