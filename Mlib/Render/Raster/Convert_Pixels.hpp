#pragma once
#include <cstdint>

namespace Mlib {
namespace Dff {

static void conv_RGBA8888_from_RGBA8888(uint8_t *out, const uint8_t *in)
{
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    out[3] = in[3];
}

static void conv_BGRA8888_from_RGBA8888(uint8_t *out, const uint8_t *in)
{
    out[2] = in[0];
    out[1] = in[1];
    out[0] = in[2];
    out[3] = in[3];
}

static void conv_RGBA8888_from_RGB888(uint8_t *out, const uint8_t *in)
{
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    out[3] = 0xFF;
}

static void conv_BGRA8888_from_RGB888(uint8_t *out, const uint8_t *in)
{
    out[2] = in[0];
    out[1] = in[1];
    out[0] = in[2];
    out[3] = 0xFF;
}

static void conv_RGB888_from_RGB888(uint8_t *out, const uint8_t *in)
{
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}

static void conv_BGR888_from_RGB888(uint8_t *out, const uint8_t *in)
{
    out[2] = in[0];
    out[1] = in[1];
    out[0] = in[2];
}

static void conv_ARGB1555_from_ARGB1555(uint8_t *out, const uint8_t *in)
{
    out[0] = in[0];
    out[1] = in[1];
}

static void conv_ARGB1555_from_RGB555(uint8_t *out, const uint8_t *in)
{
    out[0] = in[0];
    out[1] = in[1] | 0x80;
}

static void conv_RGBA5551_from_ARGB1555(uint8_t *out, const uint8_t *in)
{
    uint32_t r, g, b, a;
    a = (in[1]>>7) & 1;
    r = (in[1]>>2) & 0x1F;
    g = (in[1]&3)<<3 | ((in[0]>>5)&7);
    b = in[0] & 0x1F;
    out[0] = a | b<<1 | g<<6;
    out[1] = g>>2 | r<<3;
}

static void conv_ARGB1555_from_RGBA5551(uint8_t *out, const uint8_t *in)
{
    uint32_t r, g, b, a;
    a = in[0] & 1;
    b = (in[0]>>1) & 0x1F;
    g = (in[1]&7)<<2 | ((in[0]>>6)&3);
    r = (in[1]>>3) & 0x1F;
    out[0] = b | g<<5;
    out[1] = g>>3 | r<<2 | a<<7;
}

static void conv_RGBA8888_from_ARGB1555(uint8_t *out, const uint8_t *in)
{
    uint32_t r, g, b, a;
    a = (in[1]>>7) & 1;
    r = (in[1]>>2) & 0x1F;
    g = (in[1]&3)<<3 | ((in[0]>>5)&7);
    b = in[0] & 0x1F;
    out[0] = r*0xFF/0x1f;
    out[1] = g*0xFF/0x1f;
    out[2] = b*0xFF/0x1f;
    out[3] = a*0xFF;
}

static void conv_ABGR1555_from_ARGB1555(uint8_t *out, const uint8_t *in)
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

}
}
