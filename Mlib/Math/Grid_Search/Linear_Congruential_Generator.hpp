#pragma once
#include <Mlib/Iterator/Generator.hpp>
#include <cstdint>

namespace Mlib {

// From: Google Google Gemini
inline Generator<uint32_t> linear_congruential_generator(
    uint32_t n,
    uint32_t a = 21,    // Coprime to 1,000,000; (a-1) is a multiple of 4
    uint32_t c = 3,     // Coprime to 1,000,000
    uint32_t seed = 42)
{
    const uint32_t MODULUS = 1'000;
    if (n > MODULUS) {
        throw std::runtime_error("Number of samples requested from linear congruential generator too large");
    }
    uint32_t state = seed;
    for (uint32_t i = 0; i < n;) {
        state = (a * state + c) % MODULUS;
        if (state >= n) {
            continue;
        }
        co_yield state;
        ++i;
    }
}

}
