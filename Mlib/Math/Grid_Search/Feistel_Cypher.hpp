#pragma once
#include <Mlib/Iterator/Generator.hpp>
#include <cstdint>

namespace Mlib {

Generator<uint32_t> feistel_generator(uint32_t total_bits, uint32_t key = 123456);

}
