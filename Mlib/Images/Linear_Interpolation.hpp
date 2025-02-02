#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
class LinearInterpolator {
public:
    template <typename... TDimensions>
    TData operator () (const Array<TData>& im, TDimensions... dim) {
        TData v0 = ((1 - a0) * im(dim..., r0) + a0 * im(dim..., r1));
        return v0;
    }
    size_t r0;
    size_t r1;
    TData a0;
};

template <class TData>
bool linear_interpolation(TData rf, size_t nrows, LinearInterpolator<TData>& li) {
    if (rf < 0) {
        return false;
    }
    li.r0 = size_t(rf);
    li.r1 = li.r0 + 1;
    if (li.r1 == nrows) {
        --li.r1;
        --li.r0;
    }
    if (li.r1 >= nrows) {
        return false;
    }
    li.a0 = rf - (TData)li.r0;
    return true;
}

template <class TData>
bool linear_grayscale_interpolation(TData rf, const Array<TData>& im, TData& intensity) {
    assert(im.ndim() == 1);
    LinearInterpolator<TData> li;
    if (linear_interpolation(rf, im.shape(0), li)) {
        intensity = li(im);
        return true;
    } else {
        return false;
    }
}

template <class TData>
class InterpolationDomain1D {
public:
    InterpolationDomain1D() {}
    InterpolationDomain1D(TData xmin, TData xmax)
        : xmin_{ xmin }
        , inv_length_{ 1 / (xmax - xmin) }
    {}
    TData operator () (TData x, size_t nrows) const {
        TData rf = (x - xmin_) * inv_length_ * TData(nrows - 1);
        return rf;
    }
private:
    TData xmin_;
    TData inv_length_;
};

template <class TData>
class LinearInterpolationDomain {
public:
    LinearInterpolationDomain() {}
    LinearInterpolationDomain(TData xmin, TData xmax, const Array<TData>& im)
        : domain_{ xmin, xmax }
        , im_(im)
    {}
    bool operator () (TData x, TData& intensity) const {
        auto rf = domain_(x, im_.length());
        return linear_grayscale_interpolation(rf, im_, intensity);
    }
private:
    InterpolationDomain1D<TData> domain_;
    Array<TData> im_;
};

}
