#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Sort.hpp>

namespace Mlib {

template <class TData>
Array<TData> quantized(
    const Array<TData>& im,
    const Array<TData>& levels)
{
    Array<TData> imf = im.flattened();
    Array<TData> res(im.shape());
    for (size_t i = 0; i < imf.length(); ++i) {
        size_t am = argmin(abs(levels - im(i)));
        if (am == SIZE_MAX) {
            res(i) = NAN;
        } else {
            res(i) = levels(am);
        }
    }
    res.reshape(im.shape());
    return res;

}

}
