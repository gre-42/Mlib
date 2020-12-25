#pragma once
#include <Mlib/Stats/Histogram.hpp>

namespace Mlib {

template <class TData, class TFloat=TData>
class Cdf {
public:
    Cdf(const Array<TData>& data, size_t nbins = 10)
    : hist_{data, nbins}
    {
        const Array<size_t>& hist = hist_.hist();
        cdf_.resize(hist.shape());
        TFloat cumsum = 0;
        for (size_t i = 0; i < hist.length(); ++i) {
            cumsum += TFloat(hist(i)) / data.length();
            cdf_(i) = cumsum;
        }
    }
    const TFloat& operator () (const TData& v, bool check_bounds = false) {
        return cdf_(hist_.bin_id(v, check_bounds));
    }
    const Array<TFloat>& cdf() {
        return cdf_;
    }
    Array<TData> bins() {
        return hist_.bins();
    }
private:
    Histogram<TData> hist_;
    Array<TFloat> cdf_;
};

template <class TData, class TFloat=TData>
void cdf(const Array<TData>& data, Array<TFloat>& cdf, Array<TData>& bins, size_t nbins = 10) {
    Cdf ccdf{data, nbins};
    cdf = ccdf.cdf();
    bins = ccdf.bins();
}

}
