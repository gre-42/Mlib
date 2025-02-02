#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>
#include <stdexcept>
#include <string>

namespace Mlib {

#pragma warning( push )
#pragma warning( disable : 4103 )
#include <Mlib/Packed_Begin.hpp>
// 32-bit, 4 * 8 bits
struct Rgba32 {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
    Rgba32() {}
    Rgba32(
        unsigned char r,
        unsigned char g,
        unsigned char b,
        unsigned char a)
    :r(r), g(g), b(b), a(a) {}
    static inline Rgba32 red() {
        return Rgba32{255, 0, 0, 255};
    }
    static inline Rgba32 green() {
        return Rgba32{0, 255, 0, 255};
    }
    static inline Rgba32 blue() {
        return Rgba32{0, 0, 255, 255};
    }
    static inline Rgba32 black() {
        return Rgba32{0, 0, 0, 255};
    }
    static inline Rgba32 white() {
        return Rgba32{255, 255, 255, 255};
    }
    static inline Rgba32 nan(unsigned char a) {
        return Rgba32{255, 0, 255, a};
    }
    static inline Rgba32 gray() {
        return Rgba32{127, 127, 127, 255};
    }
    static inline Rgba32 yellow() {
        return Rgba32{255, 255, 0, 255};
    }
    static inline Rgba32 orange() {
        return Rgba32{255, 127, 0, 255};
    }
    static inline Rgba32 from_float_rgba(float r, float g, float b, float a) {
        if (std::isnan(a)) {
            THROW_OR_ABORT("from_float_rgba received NaN alpha");
        }
        if (std::isnan(r) || std::isnan(g) || std::isnan(b)) {
            if (!std::isnan(r) || !std::isnan(g) || !std::isnan(b)) {
                THROW_OR_ABORT("from_float_rgba received inconsistent NANs");
            }
            return Rgba32::nan((unsigned char)(a * 255 + 0.5f));
        }
        if (r < 0.f || g < 0.f || b < 0.f || a < 0.f) {
            THROW_OR_ABORT("from_float_rgba received value < 0");
        }
        if (r > 1.f || g > 1.f || b > 1.f || a > 1.f) {
            THROW_OR_ABORT("from_float_rgba received value > 1");
        }
        return Rgba32{
            (unsigned char)(r * 255 + 0.5f),
            (unsigned char)(g * 255 + 0.5f),
            (unsigned char)(b * 255 + 0.5f),
            (unsigned char)(a * 255 + 0.5f)};
    }
} PACKED;
#include <Mlib/Packed_End.hpp>
#pragma warning ( pop )

static_assert(sizeof(Rgba32) == 4);

}
