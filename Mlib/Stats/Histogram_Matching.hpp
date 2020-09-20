#pragma once
#include <Mlib/Stats/Cdf.hpp>
#include <Mlib/Stats/Quantile.hpp>

namespace Mlib {

template <class TData, class TDataRef, class TFloat=TData>
Array<TDataRef> histogram_matching(const Array<TData>& data, const Array<TDataRef>& data_ref, size_t nbins = 10) {
    Cdf<TData, TFloat> cdf{data, nbins};
    Quantiles quantiles{data_ref};
    Array<TData> result{data.shape()};
    for(size_t i = 0; i < data.length(); ++i) {
        result(i) = quantiles(cdf(data(i)), TFloat{0});
    }
    return result;
}

}
