#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Grid_Search/Feistel_Cypher.hpp>
#include <Mlib/Math/Grid_Search/Linear_Congruential_Generator.hpp>
#include <Mlib/Math/Unravel_Index.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <stdexcept>

namespace Mlib {

enum class GridSearchMethod {
    FEISTEL,
    LCG
};

template <size_t tndim>
Generator<FixedArray<uint32_t, tndim>> grid_search(
    const FixedArray<uint32_t, tndim>& size,
    GridSearchMethod method = GridSearchMethod::FEISTEL)
{
    switch (method) {
        case GridSearchMethod::FEISTEL: {
            uint32_t n = prod(size);
            for (uint32_t i : feistel_generator(n)) {
                auto j = unravel_index(i, size);
                co_yield j;
            }
            co_return;
        }
        case GridSearchMethod::LCG: {
            uint32_t n = prod(size);
            for (uint32_t i : linear_congruential_generator(n)) {
                auto j = unravel_index(i, size);
                co_yield j;
            }
            co_return;
        }
    }
    throw std::runtime_error("Unknown grid search method");
}

}
