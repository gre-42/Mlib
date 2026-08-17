#pragma once
#include <cstdint>
#include <vector>

namespace Mlib {

std::vector<uint32_t> transmission_lut(
    const std::vector<uint32_t>& step_exponents,
    uint32_t lut_size_exponent);

class TransmissionLut {
public:
    TransmissionLut(const std::vector<uint32_t>& lut);
    uint32_t operator () ();
private:
    const std::vector<uint32_t>& lut_;
    uint32_t i_;
};

}
