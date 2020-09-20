#pragma once
#include <Mlib/Stats/Histogram.hpp>

namespace Mlib {

template <class TData, class TFloat=TData>
void cdf(const Array<TData>& data, Array<TFloat>& cdf, Array<TData>& bins, size_t nbins = 10) {
    Array<size_t> hist;
    histogram(data, hist, bins, nbins);
    cdf.resize(hist.shape());
    TFloat cumsum = 0;
    for(size_t i = 0; i < hist.length(); ++i) {
        cumsum += TFloat(hist(i)) / data.length();
        cdf(i) = cumsum;
    }
}

}
