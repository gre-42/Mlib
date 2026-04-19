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
// 16-bit, 2 * 8 bits
struct Ia16 {
    unsigned char i;
    unsigned char a;
    Ia16() {}
    Ia16(
        unsigned char i,
        unsigned char a)
    :i(i), a(a) {}
    static inline Ia16 black() {
        return Ia16{0, 255};
    }
    static inline Ia16 white() {
        return Ia16{255, 255};
    }
    static inline Ia16 nan(unsigned char a) {
        return Ia16{123, a};
    }
    static inline Ia16 gray() {
        return Ia16{127, 255};
    }
    static inline Ia16 from_float_ia(float i, float a) {
        if (std::isnan(a)) {
            throw std::runtime_error("from_float_ia received NaN alpha");
        }
        if (std::isnan(i)) {
            return Ia16::nan((unsigned char)std::round(a * 255));
        }
        if (i < 0.f || a < 0.f) {
            throw std::runtime_error("from_float_ia received value < 0");
        }
        if (i > 1.f || a > 1.f) {
            throw std::runtime_error("from_float_ia received value > 1");
        }
        return Ia16{
            (unsigned char)(std::round(i * 255)),
            (unsigned char)(std::round(a * 255))};
    }
    static inline Ia16 from_float_ia(const FixedArray<float, 2>& ia) {
        return from_float_ia(ia(0), ia(1));
    }
} PACKED;
#include <Mlib/Misc/Packed_End.hpp>
PRAGMA_MSC_WARNING_POP

static_assert(sizeof(Ia16) == 2);

}
