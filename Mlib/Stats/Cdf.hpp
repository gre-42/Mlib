#pragma once
#include <Mlib/Stats/Histogram.hpp>

namespace Mlib {

template <class TData, class TFloat>
class Cdf {
public:
    Cdf(const Array<TData>& data, size_t nbins = 10)
        : hist_{data, nbins}
    {
        if (hist_.nobservations == 0) {
            throw std::runtime_error("CDF received no observations");
        }
        const Array<size_t>& hist = hist_.hist();
        cdf_.resize(hist.shape());
        TFloat cumsum = 0;
        for (size_t i = 0; i < hist.length(); ++i) {
            cumsum += TFloat(hist(i)) / TFloat(hist_.nobservations);
            cdf_(i) = std::min((TFloat)1, cumsum);
        }
    }
    const TFloat& operator () (const TData& v, OutOfBoundsBehavior out_of_bounds_behavior) const {
        return cdf_(hist_.bin_id(v, out_of_bounds_behavior));
    }
    const Array<TFloat>& cdf() const {
        return cdf_;
    }
    Array<TData> bin_boundaries() const {
        return hist_.bin_boundaries();
    }
    Array<TData> bin_centers() const {
        return hist_.bin_centers();
    }
    size_t nbins() const {
        return hist_.nbins();
    }
private:
    Histogram<TData> hist_;
    Array<TFloat> cdf_;
};

template <class TData, class TFloat>
void cdf(const Array<TData>& data, Array<TFloat>& cdf, Array<TData>& bins, size_t nbins = 10) {
    Cdf<TData, TFloat> ccdf{data, nbins};
    cdf = ccdf.cdf();
    bins = ccdf.bin_centers();
}

}
