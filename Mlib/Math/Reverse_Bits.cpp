#include "Reverse_Bits.hpp"

using namespace Mlib;

// From: https://rosettacode.org/wiki/Fast_Fourier_transform#C.2B.2B
uint32_t Mlib::reverse_bits(uint32_t a, uint32_t nbits) {
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
