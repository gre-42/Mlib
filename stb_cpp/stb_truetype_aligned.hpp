#pragma once
#include <cstdint>
#include <stb/stb_truetype.h>
#include <unordered_map>

float stbtt_BakeFontBitmap_get_y0(
    unsigned char *data, int offset,                        // font location (use offset=0 for plain .ttf)
    float pixel_height,                                     // height of font in pixels
    unsigned char *pixels, int pw, int ph,                  // bitmap to be filled in
    const std::unordered_map<char32_t, uint32_t>& chars,    // characters to bake
    stbtt_bakedchar *chardata);
