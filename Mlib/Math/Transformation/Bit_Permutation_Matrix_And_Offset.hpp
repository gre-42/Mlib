#pragma once
#include <Mlib/Initialization/Uninitialized.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <array>
#include <cstdint>

namespace Mlib {

template <uint32_t W>
class BitPermutationMatrixAndOffset {
public:
    template <class RandomFunc>
    static BitPermutationMatrixAndOffset from_random(uint32_t size, RandomFunc& r) {
        BitPermutationMatrixAndOffset result(uninitialized);
        for (uint32_t i = 0; i < size; ++i) {
            result.p_[i] = (1u << i);
        }
        for (uint32_t i = size; i < W; ++i) {
            result.p_[i] = 0;
        }
        std::shuffle(result.p_.begin(), result.p_.begin() + size, r);
        result.offset_ = r();
        if (size < 32) {
            result.offset_ &= ((1u << size) - 1u);
        }
        result.size_ = size;
        return result;
    }
    BitPermutationMatrixAndOffset(Uninitialized)
    {}
    BitPermutationMatrixAndOffset(
        const std::array<uint32_t, W>& p,
        uint32_t offset,
        uint32_t size)
        : p_{p}
        , offset_{offset}
        , size_{size}
    {}
    uint32_t operator * (uint32_t rhs) const {
        uint32_t result = offset_;
        for (auto&& [b, r] : tenumerate<uint32_t>(p_)) {
            if (rhs & (1u << b)) {
                result ^= r;  // Linear transformation via XOR accumulation
            }
        }
        if (size_ < 32) {
            result &= ((1u << size_) - 1u);
        }
        return result;
    }
private:
    std::array<uint32_t, W> p_;
    uint32_t offset_;
    uint32_t size_;
};

}
