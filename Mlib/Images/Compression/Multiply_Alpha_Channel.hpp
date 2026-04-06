#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Transform/Upsample.hpp>
#include <stdexcept>

namespace Mlib {

template <class T>
Array<T> multiply_alpha_channel(const Array<T>& color, const Array<T>& alpha) {
    if (color.ndim() != 3) {
        throw std::runtime_error("Color image does not have 3 dimensions");
    }
    if (alpha.ndim() != 2) {
        throw std::runtime_error("Alpha image does not have 2 dimensions");
    }
    if (color.shape(0) == 0) {
        throw std::runtime_error("Color image has no channels");
    }
    auto result = Array<T>{ArrayShape{color.shape(0), color.shape(1), color.shape(2)}};
    auto upsampled = Upsample{
        alpha.reshaped(ArrayShape{1, alpha.shape(0), alpha.shape(1)}),
        color.shape(2),
        color.shape(1)};
    for (size_t h = 0; h < color.shape(0); ++h) {
        result[h] = color[h];
    }
    for (size_t r = 0; r < color.shape(1); ++r) {
        for (size_t c = 0; c < color.shape(2); ++c) {
            result(color.shape(0) - 1, r, c) *= upsampled(r, c)[0];
        }
    }
    return result;
}

}
