#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>
#include <stdexcept>

namespace Mlib {

#pragma warning( push )
#pragma warning( disable : 4103 )
#include <Mlib/Packed_Begin.hpp>
// 16-bit, 2 * 8 bytes
struct Bgr565 {
    uint16_t b:5;
    uint16_t g:6;
    uint16_t r:5;
    Bgr565() {}
    Bgr565(
        uint16_t r,
        uint16_t g,
        uint16_t b)
    :b(b), g(g), r(r) {}
    static inline Bgr565 red() {
        return Bgr565{
            (1 << 5) - 1,
            0,
            0};
    }
    static inline Bgr565 green() {
        return Bgr565{
            0,
            (1 << 6) - 1,
            0};
    }
    static inline Bgr565 blue() {
        return Bgr565{
            0,
            0,
            (1 << 5) - 1};
    }
    static inline Bgr565 black() {
        return Bgr565{0, 0, 0};
    }
    static inline Bgr565 white() {
        return Bgr565{
            (1 << 5) - 1,
            (1 << 6) - 1,
            (1 << 5) - 1};
    }
    static inline Bgr565 nan() {
        return Bgr565{
            (1 << 5) - 1,
            0,
            (1 << 5) - 1};
    }
    static inline Bgr565 from_float_grayscale(float grayscale) {
        if (std::isnan(grayscale)) {
            return Bgr565::nan();
        }
        // consider using grayscale.clip(0, 1) if this fails
        if (grayscale < 0) {
            THROW_OR_ABORT("Bgr565Bitmap::from_float_grayscale received " + std::to_string(grayscale) + "<0");
        }
        if (grayscale > 1) {
            THROW_OR_ABORT("Bgr565Bitmap::from_float_grayscale received " + std::to_string(grayscale) + ">1");
        }
        return Bgr565{
            (uint16_t)(grayscale * ((1 << 5) - 1) + 0.5f),
            (uint16_t)(grayscale * ((1 << 6) - 1) + 0.5f),
            (uint16_t)(grayscale * ((1 << 5) - 1) + 0.5f)};
    }
    static inline Bgr565 from_float_rgb(const Array<float>& rgb) {
        if (any(Mlib::isnan(rgb))) {
            if (!all(Mlib::isnan(rgb))) {
                THROW_OR_ABORT("Bgr565Bitmap::from_float_rgb received inconsistent NANs");
            }
            return Bgr565::nan();
        }
        assert(all(rgb.shape() == ArrayShape{3}));
        if (any(rgb < 0.f)) {
            THROW_OR_ABORT("Bgr565Bitmap::from_float_rgb received " + rgb.str() + "<0");
        }
        if (any(rgb > 1.f)) {
            THROW_OR_ABORT("Bgr565Bitmap::from_float_rgb received " + rgb.str() + ">1");
        }
        return Bgr565{
            (uint16_t)(rgb(0) * ((1 << 5) - 1) + 0.5f),
            (uint16_t)(rgb(1) * ((1 << 6) - 1) + 0.5f),
            (uint16_t)(rgb(2) * ((1 << 5) - 1) + 0.5f)};
    }
    static inline Bgr565 from_float_rgb(float r, float g, float b) {
        if (std::isnan(r) || std::isnan(g) || std::isnan(b)) {
            if (!std::isnan(r) || !std::isnan(g) || !std::isnan(b)) {
                THROW_OR_ABORT("Bgr565Bitmap::from_float_rgb received inconsistent NANs");
            }
            return Bgr565::nan();
        }
        if (r < 0.f || g < 0.f || b < 0.f) {
            THROW_OR_ABORT("Bgr565Bitmap::from_float_rgb received value < 0");
        }
        if (r > 1.f || g > 1.f || b > 1.f) {
            THROW_OR_ABORT("Bgr565Bitmap::from_float_rgb received value > 1");
        }
        return Bgr565{
            (uint16_t)(r * ((1 << 5) - 1) + 0.5f),
            (uint16_t)(g * ((1 << 6) - 1) + 0.5f),
            (uint16_t)(b * ((1 << 5) - 1) + 0.5f)};
    }
} PACKED;
#include <Mlib/Packed_End.hpp>
#pragma warning ( pop )

}
