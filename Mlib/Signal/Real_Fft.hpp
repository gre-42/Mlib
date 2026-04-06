#pragma once
#include <Mlib/Signal/Fft.hpp>

namespace Mlib {

template <class TFloat>
Array<std::complex<TFloat>> real_fft(const Array<TFloat>& x) {
    for (size_t axis = 0; axis < x.ndim(); ++axis) {
        if (x.shape(axis) == 0) {
            return Array<std::complex<TFloat>>{x.shape()};
        }
        if (x.shape(axis) % 2 != 0) {
            throw std::runtime_error("shape is not not even");
        }
    }
    auto res0 = fft(x.template casted<std::complex<TFloat>>());
    auto res1 = Array<std::complex<TFloat>>{(x.shape() - 2) / 2 + 2};
    res1.shape().foreach([&](const auto& index){
        res1(index) = res0(index);
    });
    return res1;
}

template <class TFloat>
Array<TFloat> real_ifft(const Array<std::complex<TFloat>>& x) {
    // c = (s - 2) / 2 + 2
    // => s = (c - 2) * 2 + 2
    for (size_t axis = 0; axis < x.ndim(); ++axis) {
        if (x.shape(axis) == 0) {
            return Array<TFloat>{x.shape()};
        }
        if (x.shape(axis) % 2 != 1) {
            throw std::runtime_error("shape is not not odd");
        }
    }
    auto x1 = Array<std::complex<TFloat>>{(x.shape() - 2) * 2 + 2};
    x.shape().foreach([&](const auto& index){
        x1(index) = x(index);
    });
    ArrayShape id0(x.ndim());
    foreach(x.shape(), x1.shape(), [&](const auto& index){
        id0 = x1.shape();
        id0 -= index;
        x1(index) = std::conj(x(id0));
    });
    return real(ifft(x1));
}

}
