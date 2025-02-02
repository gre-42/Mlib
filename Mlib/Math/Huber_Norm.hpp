#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
Array<TData> huber_norm_multichannel(const Array<TData>& x, TData epsilon) {
    Array<TData> res = zeros<TData>(x.shape().erased_first());
    Array<TData> rf = res.flattened();
    Array<TData> xf = x.columns_as_1D();
    for (size_t i = 0; i < xf.shape(1); ++i) {
        TData l22 = 0;
        for (size_t h = 0; h < xf.shape(0); ++h) {
            l22 += squared(xf(h, i));
        }
        if (l22 < squared(epsilon)) {
            rf(i) = l22 / (2 * epsilon);
        } else {
            rf(i) = std::sqrt(l22) - epsilon / 2;
        }
    }
    return res;
}

template <class TData>
Array<TData> huber_norm(const Array<TData>& x, TData epsilon, bool multichannel) {
    Array<TData> res = huber_norm_multichannel(
        multichannel ? x : x.reshaped(ArrayShape{1}.concatenated(x.shape())),
        epsilon);
    return multichannel ? res : res.reshaped(x.shape());
}

}
