#include "Transmission_Scheduler.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Misc/Reverse_Bits.hpp>
#include <bit>
#include <stdexcept>

using namespace Mlib;

std::vector<uint32_t> Mlib::transmission_lut(
    const std::vector<uint32_t>& step_exponents,
    uint32_t lut_size_exponent)
{
    uint32_t n = (1 << lut_size_exponent);
    uint32_t w = integral_cast<uint32_t>(std::bit_width(step_exponents.size()));
    std::vector<uint32_t> result(n, 0);
    for (const auto& [x, e] : tenumerate<uint32_t>(step_exponents)) {
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
                result[j] |= (1 << x);
            }
            break;
        }
    }
    return result;
}

TransmissionLut::TransmissionLut(const std::vector<uint32_t>& lut)
    : lut_{ lut }
    , i_{ 0 }
{}

uint32_t TransmissionLut::operator () () {
    return lut_[(i_++) & (lut_.size() - 1)];
}
