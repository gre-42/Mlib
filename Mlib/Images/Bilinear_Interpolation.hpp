#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
class BilinearInterpolator {
public:
    TData interpolate_grayscale(const Array<TData>& im) {
        assert(im.ndim() == 2);
        float v00 = ((1 - a0) * im(r0, c0) + a0 * im(r1, c0));
        float v01 = ((1 - a0) * im(r0, c1) + a0 * im(r1, c1));
        return (1 - a1) * v00 + a1 * v01;
    }
    TData interpolate_multichannel(const Array<TData>& im, size_t dim) {
        assert(im.ndim() == 3);
        float v00 = ((1 - a0) * im(dim, r0, c0) + a0 * im(dim, r1, c0));
        float v01 = ((1 - a0) * im(dim, r0, c1) + a0 * im(dim, r1, c1));
        return (1 - a1) * v00 + a1 * v01;
    }
    size_t r0;
    size_t r1;
    size_t c0;
    size_t c1;
    float a0;
    float a1;
};

template <class TData>
bool bilinear_interpolation(TData rf, TData cf, const ArrayShape& shape, BilinearInterpolator<TData>& bi) {
    assert(shape.ndim() == 2);
    if (rf < 0 || cf < 0) {
        return false;
    }
    bi.r0 = size_t(rf);
    bi.r1 = bi.r0 + 1;
    bi.c0 = size_t(cf);
    bi.c1 = bi.c0 + 1;
    if (bi.r1 == shape(0)) {
        --bi.r1;
        --bi.r0;
    }
    if (bi.c1 == shape(1)) {
        --bi.c1;
        --bi.c0;
    }
    if (bi.r1 >= shape(0) || bi.c1 >= shape(1)) {
        return false;
    }
    bi.a0 = (rf - bi.r0);
    bi.a1 = (cf - bi.c0);
    return true;
}

template <class TData>
bool bilinear_grayscale_interpolation(TData rf, TData cf, const Array<TData>& im, TData& intensity) {
    BilinearInterpolator<TData> bi;
    if (bilinear_interpolation(rf, cf, im.shape(), bi)) {
        intensity = bi.interpolate_grayscale(im);
        return true;
    } else {
        return false;
    }
}

}
