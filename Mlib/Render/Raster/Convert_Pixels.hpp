#pragma once
#include <cstdint>

namespace Mlib {
namespace Dff {

inline void conv_RGBA8888_from_RGBA8888(uint8_t *out, const uint8_t *in)
{
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    out[3] = in[3];
}

inline void conv_BGRA8888_from_RGBA8888(uint8_t *out, const uint8_t *in)
{
    out[2] = in[0];
    out[1] = in[1];
    out[0] = in[2];
    out[3] = in[3];
}

inline void conv_RGBA8888_from_RGB888(uint8_t *out, const uint8_t *in)
{
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    out[3] = 0xFF;
}

inline void conv_BGRA8888_from_RGB888(uint8_t *out, const uint8_t *in)
{
    out[2] = in[0];
    out[1] = in[1];
    out[0] = in[2];
    out[3] = 0xFF;
}

inline void conv_RGB888_from_RGB888(uint8_t *out, const uint8_t *in)
{
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}

inline void conv_BGR888_from_RGB888(uint8_t *out, const uint8_t *in)
{
    out[2] = in[0];
    out[1] = in[1];
    out[0] = in[2];
}

inline void conv_ARGB1555_from_ARGB1555(uint8_t *out, const uint8_t *in)
{
    out[0] = in[0];
    out[1] = in[1];
}

inline void conv_ARGB1555_from_RGB555(uint8_t *out, const uint8_t *in)
{
    out[0] = in[0];
    out[1] = in[1] | 0x80;
}

inline void conv_RGBA5551_from_ARGB1555(uint8_t *out, const uint8_t *in)
{
    uint32_t r, g, b, a;
    a = (in[1]>>7) & 1;
    r = (in[1]>>2) & 0x1F;
    g = (in[1]&3u)<<3 | ((in[0]>>5)&7u);
    b = in[0] & 0x1F;
    out[0] = a | b<<1 | g<<6;
    out[1] = g>>2 | r<<3;
}

inline void conv_ARGB1555_from_RGBA5551(uint8_t *out, const uint8_t *in)
{
    uint32_t r, g, b, a;
    a = in[0] & 1;
    b = (in[0]>>1) & 0x1F;
    g = (in[1]&7u)<<2 | ((in[0]>>6)&3u);
    r = (in[1]>>3) & 0x1Fu;
    out[0] = b | g<<5;
    out[1] = g>>3 | r<<2 | a<<7;
}

inline void conv_RGBA8888_from_ARGB1555(uint8_t *out, const uint8_t *in)
{
    uint32_t r, g, b, a;
    a = (in[1]>>7) & 1;
    r = (in[1]>>2) & 0x1F;
    g = (in[1]&3u)<<3 | ((in[0]>>5)&7u);
    b = in[0] & 0x1F;
    out[0] = r*0xFFu/0x1f;
    out[1] = g*0xFFu/0x1f;
    out[2] = b*0xFFu/0x1f;
    out[3] = a*0xFFu;
}

inline void conv_ABGR1555_from_ARGB1555(uint8_t *out, const uint8_t *in)
{
    uint32_t r, b;
    r = (in[1]>>2) & 0x1F;
    b = in[0] & 0x1F;
    out[1] = (in[1]&0x83) | b<<2;
    out[0] = (in[0]&0xE0) | r;
}

// some swaps are the same, so these are just more descriptive names
inline void conv_RGBA8888_from_BGRA8888(uint8_t *out, const uint8_t *in) { conv_BGRA8888_from_RGBA8888(out, in); }
inline void conv_RGB888_from_BGR888(uint8_t *out, const uint8_t *in) { conv_BGR888_from_RGB888(out, in); }
inline void conv_ARGB1555_from_ABGR1555(uint8_t *out, const uint8_t *in) { conv_ABGR1555_from_ARGB1555(out, in); }

inline void expandPal4(uint8_t *dst, uint32_t dststride, const uint8_t *src, uint32_t srcstride, uint32_t w, uint32_t h)
{
    uint32_t x, y;
    for(y = 0; y < h; y++)
        for(x = 0; x < w/2; x++){
            dst[y*dststride + x*2 + 0] = src[y*srcstride + x] & 0xF;
            dst[y*dststride + x*2 + 1] = src[y*srcstride + x] >> 4;
        }
}

inline void compressPal4(uint8_t *dst, uint32_t dststride, const uint8_t *src, uint32_t srcstride, uint32_t w, uint32_t h)
{
    uint32_t x, y;
    for(y = 0; y < h; y++)
        for(x = 0; x < w/2; x++)
            dst[y*dststride + x] = src[y*srcstride + x*2 + 0] | src[y*srcstride + x*2 + 1] << 4;
}

inline void expandPal4_BE(uint8_t *dst, uint32_t dststride, const uint8_t *src, uint32_t srcstride, uint32_t w, uint32_t h)
{
    uint32_t x, y;
    for(y = 0; y < h; y++)
        for(x = 0; x < w/2; x++){
            dst[y*dststride + x*2 + 1] = src[y*srcstride + x] & 0xF;
            dst[y*dststride + x*2 + 0] = src[y*srcstride + x] >> 4;
        }
}

inline void compressPal4_BE(uint8_t *dst, uint32_t dststride, const uint8_t *src, uint32_t srcstride, uint32_t w, uint32_t h)
{
    uint32_t x, y;
    for(y = 0; y < h; y++)
        for(x = 0; x < w/2; x++)
            dst[y*dststride + x] = src[y*srcstride + x*2 + 1] | src[y*srcstride + x*2 + 0] << 4;
}

inline void copyPal8(uint8_t *dst, uint32_t dststride, const uint8_t *src, uint32_t srcstride, uint32_t w, uint32_t h)
{
    uint32_t x, y;
    for(y = 0; y < h; y++)
        for(x = 0; x < w; x++)
            dst[y*dststride + x] = src[y*srcstride + x];
}

}
}
