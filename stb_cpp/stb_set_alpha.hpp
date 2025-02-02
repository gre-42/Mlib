#pragma once

void stb_set_alpha(
    const unsigned char* rgb,
    const unsigned char* alpha,
    unsigned char* rgba,
    int width,
    int height,
    int alpha_width,
    int alpha_height);
