#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>
#include <stdexcept>
#include <string>

namespace Mlib {

#pragma warning( push )
#pragma warning( disable : 4103 )
#include <Mlib/Packed_Begin.hpp>
// 24-bit, 3 * 8 bits
struct Rgb24 {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    Rgb24() {}
    Rgb24(
        unsigned char r,
        unsigned char g,
        unsigned char b)
    :r(r), g(g), b(b) {}
    static inline Rgb24 red() {
        return Rgb24{255, 0, 0};
    }
    static inline Rgb24 green() {
        return Rgb24{0, 255, 0};
    }
    static inline Rgb24 blue() {
        return Rgb24{0, 0, 255};
    }
    static inline Rgb24 black() {
        return Rgb24{0, 0, 0};
    }
    static inline Rgb24 white() {
        return Rgb24{255, 255, 255};
    }
    static inline Rgb24 nan() {
        return Rgb24{255, 0, 255};
    }
    static inline Rgb24 gray() {
        return Rgb24{127, 127, 127};
    }
    static inline Rgb24 yellow() {
        return Rgb24{255, 255, 0};
    }
    static inline Rgb24 orange() {
        return Rgb24{255, 127, 0};
    }
    static inline Rgb24 from_float_grayscale(float grayscale) {
        if (std::isnan(grayscale)) {
            return Rgb24::nan();
        }
        // consider using grayscale.clip(0, 1) if this fails
        if (grayscale < 0) {
            THROW_OR_ABORT("PpmImage::from_float_grayscale received " + std::to_string(grayscale) + "<0");
        }
        if (grayscale > 1) {
            THROW_OR_ABORT("PpmImage::from_float_grayscale received " + std::to_string(grayscale) + ">1");
        }
        return Rgb24{
            (unsigned char)(grayscale * 255 + 0.5f),
            (unsigned char)(grayscale * 255 + 0.5f),
            (unsigned char)(grayscale * 255 + 0.5f)};
    }
    static inline Rgb24 from_float_rgb(float r, float g, float b) {
        if (std::isnan(r) || std::isnan(g) || std::isnan(b)) {
            if (!std::isnan(r) || !std::isnan(g) || !std::isnan(b)) {
                THROW_OR_ABORT("PpmImage::from_float_rgb received inconsistent NANs");
            }
            return Rgb24::nan();
        }
        if (r < 0.f || g < 0.f || b < 0.f) {
            THROW_OR_ABORT("PpmImage::from_float_rgb received value < 0");
        }
        if (r > 1.f || g > 1.f || b > 1.f) {
            THROW_OR_ABORT("PpmImage::from_float_rgb received value > 1");
        }
        return Rgb24{
            (unsigned char)(r * 255 + 0.5f),
            (unsigned char)(g * 255 + 0.5f),
            (unsigned char)(b * 255 + 0.5f)};
    }
} PACKED;
#include <Mlib/Packed_End.hpp>
#pragma warning ( pop )

static_assert(sizeof(Rgb24) == 3);

}
