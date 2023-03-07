#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
class BilinearInterpolator {
public:
    template <typename... TDimensions>
    TData operator () (const Array<TData>& im, TDimensions... dim) {
        TData v00 = ((1 - a0) * im(dim..., r0, c0) + a0 * im(dim..., r1, c0));
        TData v01 = ((1 - a0) * im(dim..., r0, c1) + a0 * im(dim..., r1, c1));
        return (1 - a1) * v00 + a1 * v01;
    }
    size_t r0;
    size_t r1;
    size_t c0;
    size_t c1;
    TData a0;
    TData a1;
};

template <class TData>
bool bilinear_interpolation(TData rf, TData cf, size_t nrows, size_t ncols, BilinearInterpolator<TData>& bi) {
    if (rf < 0 || cf < 0) {
        return false;
    }
    bi.r0 = size_t(rf);
    bi.r1 = bi.r0 + 1;
    bi.c0 = size_t(cf);
    bi.c1 = bi.c0 + 1;
    if (bi.r1 == nrows) {
        --bi.r1;
        --bi.r0;
    }
    if (bi.c1 == ncols) {
        --bi.c1;
        --bi.c0;
    }
    if (bi.r1 >= nrows || bi.c1 >= ncols) {
        return false;
    }
    bi.a0 = rf - (TData)bi.r0;
    bi.a1 = cf - (TData)bi.c0;
    return true;
}

template <class TData>
bool bilinear_grayscale_interpolation(TData rf, TData cf, const Array<TData>& im, TData& intensity) {
    assert(im.ndim() == 2);
    BilinearInterpolator<TData> bi;
    if (bilinear_interpolation(rf, cf, im.shape(0), im.shape(1), bi)) {
        intensity = bi(im);
        return true;
    } else {
        return false;
    }
}

}
