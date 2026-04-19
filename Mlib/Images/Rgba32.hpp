#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Misc/Pragma_Msc.hpp>
#include <cmath>
#include <stdexcept>
#include <string>

namespace Mlib {

PRAGMA_MSC_WARNING_PUSH
PRAGMA_MSC_WARNING_DISABLE(4103)
#include <Mlib/Misc/Packed_Begin.hpp>
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
            throw std::runtime_error("from_float_rgba received NaN alpha");
        }
        if (std::isnan(r) || std::isnan(g) || std::isnan(b)) {
            if (!std::isnan(r) || !std::isnan(g) || !std::isnan(b)) {
                throw std::runtime_error("from_float_rgba received inconsistent NANs");
            }
            return Rgba32::nan((unsigned char)std::round(a * 255));
        }
        if (r < 0.f || g < 0.f || b < 0.f || a < 0.f) {
            throw std::runtime_error("from_float_rgba received value < 0");
        }
        if (r > 1.f || g > 1.f || b > 1.f || a > 1.f) {
            throw std::runtime_error("from_float_rgba received value > 1");
        }
        return Rgba32{
            (unsigned char)(std::round(r * 255)),
            (unsigned char)(std::round(g * 255)),
            (unsigned char)(std::round(b * 255)),
            (unsigned char)(std::round(a * 255))};
    }
    static inline Rgba32 from_float_rgba(const FixedArray<float, 4>& rgba) {
        return from_float_rgba(rgba(0), rgba(1), rgba(2), rgba(3));
    }
} PACKED;
#include <Mlib/Misc/Packed_End.hpp>
PRAGMA_MSC_WARNING_POP

static_assert(sizeof(Rgba32) == 4);

}
