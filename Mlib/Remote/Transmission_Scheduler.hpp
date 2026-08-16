#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace Mlib {

// From: https://rosettacode.org/wiki/Fast_Fourier_transform#C.2B.2B
uint32_t reverse_bits(uint32_t a, uint32_t nbits) {
    if (nbits == 0) {
        return 0;
    }
    uint32_t b = a;
    b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
    b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
    b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
    b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
    b = ((b >> 16) | (b << 16)) >> (32 - nbits);
    return b;
}

std::vector<uint32_t> transmission_lut(
    const std::vector<uint32_t>& step_exponents,
    uint32_t lut_size_exponent)
{
    uint32_t n = (1 << lut_size_exponent);
    uint32_t w = integral_cast<uint32_t>(std::bit_width(step_exponents.size()));
    std::vector<uint32_t> result(n, 0);
    for (uint32_t e : step_exponents) {
        uint32_t step = (1 << e);
        for (uint32_t i = 0; i < step; ++i) {
            uint32_t offset = reverse_bits(i, w);
            if (offset >= n) {
                throw std::runtime_error("transmission_lut: offset exceeds array size");
            }
            if ((i != step - 1) && (result[offset] != 0)) {
                continue;
            }
            for (uint32_t j = offset; j < n; j += step) {
                result[j] |= (1 << i);
            }
            break;
        }
    }
    return result;
}

class TransmissionLut {
public:
    TransmissionLut(
        const std::vector<uint32_t>& step_exponents,
        uint32_t lut_size_exponent)
        : lut_{ transmission_lut(step_exponents, lut_size_exponent) }
        , i_{ 0 }
    {}
    uint32_t operator () () {
        return lut_[(i_++) & (lut_.size() - 1)];
    }
private:
    std::vector<uint32_t> lut_;
    uint32_t i_;
};

}
