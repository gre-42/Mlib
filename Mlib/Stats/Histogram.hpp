#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Stats/Min_Max.hpp>

namespace Mlib {

template <class TData>
class Histogram {
public:
    Histogram(const Array<TData>& data, size_t nbins = 10) {
        assert(nbins > 1);
        Array<TData> fdata = data[isfinite(data)];
        mi_ = min(fdata);
        ma_ = max(fdata);
        // TData h = (ma - mi) / (nbins - 1);
        // Array<TData> boundaries = linspace(mi - h / 2, ma + h / 2, nbins + 1);
        hist_ = zeros<size_t>(ArrayShape{nbins});
        for(const TData& v : fdata.flat_iterable()) {
            // for(size_t j = 1; j < boundaries.length(); ++j) {
            //     if (const TData& v = fdata(i); v >= boundaries(j - 1) && v < boundaries(j)) {
            //         ++hist(j - 1);
            //     }
            // }
            ++hist_(bin_id(v));
        }
    }
    const Array<size_t>& hist() {
        return hist_;
    }
    Array<TData> bins() {
        return linspace(mi_, ma_, hist_.length());
    }
    size_t bin_id(const TData& data) {
        return ((hist_.length() - 1) * (data - mi_)) / (ma_ - mi_);
    }
private:
    Array<size_t> hist_;
    TData mi_;
    TData ma_;
};

template <class TData>
void histogram(const Array<TData>& data, Array<size_t>& hist, Array<TData>& bins, size_t nbins = 10) {
    Histogram hm{data, nbins};
    hist = hm.hist();
    bins = hm.bins();
}

}
