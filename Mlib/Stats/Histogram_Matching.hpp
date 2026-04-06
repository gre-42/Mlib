#pragma once
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Ols.hpp>
#include <Mlib/Stats/Cdf.hpp>
#include <Mlib/Stats/Quantile.hpp>

namespace Mlib {

template <class TData, class TDataRef, class TFloat>
class HistogramMatching {
public:
    HistogramMatching(const Array<TData>& data, const Array<TDataRef>& data_ref, size_t nbins)
        : cdf_{data, nbins}
        , quantiles_{data_ref}
    {}
    const TData& operator () (const TData& v, OutOfBoundsBehavior out_of_bounds_behavior) const {
        return quantiles_(cdf_(v, out_of_bounds_behavior));
    }
    Array<TData> x() const {
        return cdf_.bin_centers();
    }
    Array<TData> y() const {
        auto vbins = cdf_.bin_centers();
        Array<TData> result(vbins.shape());
        for (const auto& [i, b] : enumerate(vbins.flat_iterable())) {
            result(i) = (*this)(b, OutOfBoundsBehavior::THROW);
        }
        return result;
    }
    Ols<TFloat> ols() const {
        return {x(), y()};
    }
    size_t nbins() const {
        return cdf_.nbins();
    }
private:
    Cdf<TData, TFloat> cdf_;
    Quantiles<TData> quantiles_;
};

template <class TData, class TDataRef, class TFloat>
struct HistogramAndMatched {
    HistogramAndMatched(
        const Array<TData>& data,
        const Array<TDataRef>& data_ref,
        size_t nbins,
        OutOfBoundsBehavior out_of_bounds_behavior)
        : hm{data, data_ref, nbins}
    {
        matched = Array<TData>{data.shape()};
        auto df = data.flattened();
        auto mf = matched.flattened();
        for (size_t i = 0; i < mf.length(); ++i) {
            mf(i) = hm(df(i), out_of_bounds_behavior);
        }
    }
    HistogramMatching<TData, TDataRef, TFloat> hm;
    Array<TDataRef> matched;
};

}
