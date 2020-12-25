#pragma once
#include <Mlib/Stats/Cdf.hpp>
#include <Mlib/Stats/Quantile.hpp>

namespace Mlib {

template <class TData, class TDataRef, class TFloat=TData>
class HistogramMatching {
public:
    HistogramMatching(const Array<TData>& data, const Array<TDataRef>& data_ref, size_t nbins = 10)
    : cdf_{data, nbins},
      quantiles_{data_ref}
    {}
    const TData& operator () (const TData& v, bool check_bounds = false) {
        return quantiles_(cdf_(v, check_bounds), TFloat{0});
    }
private:
    Cdf<TData, TFloat> cdf_;
    Quantiles<TData> quantiles_;
};

template <class TData, class TDataRef, class TFloat=TData>
Array<TDataRef> histogram_matching(const Array<TData>& data, const Array<TDataRef>& data_ref, size_t nbins = 10) {
    HistogramMatching<TData, TDataRef, TFloat> hm{data, data_ref, nbins};
    Array<TData> result{data.shape()};
    for (size_t i = 0; i < data.length(); ++i) {
        result(i) = hm(data(i));
    }
    return result;
}

}
