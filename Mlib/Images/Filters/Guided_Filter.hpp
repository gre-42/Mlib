#pragma once
#include <Mlib/Images/Filters/Box_Filter.hpp>

namespace Mlib {

template <class TData, class TFilter>
Array<TData> guided_filter(
    const Array<TData>& guidance,
    const Array<TData>& image,
    const TData& eps,
    const TFilter& w)
{
    const auto& I = guidance;
    const auto& p = image;
    auto pm = w(p);
    auto mu = w(I);
    auto s2 = w(squared(I)) - squared(w(I));
    auto a = (w(I * p) - mu * pm) / (s2 + eps);
    auto b = pm - a * mu;
    auto q = w(a) * I + w(b);
    return q;
}

template <class TData>
Array<TData> guided_filter(
    const Array<TData>& guidance,
    const Array<TData>& image,
    const ArrayShape& size,
    const TData& eps)
{
    return guided_filter(
        guidance,
        image,
        eps,
        [&size](const Array<TData>& x) { return box_filter_nans_as_zeros_NWE(x, size); });
}

}
