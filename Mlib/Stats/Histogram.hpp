#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Stats/Min_Max.hpp>

namespace Mlib {

template <class TData>
void histogram(const Array<TData>& data, Array<size_t>& hist, Array<TData>& bins, size_t nbins = 10) {
    assert(nbins > 1);
    Array<TData> fdata = data[isfinite(data)];
    TData mi = min(fdata);
    TData ma = max(fdata);
    // TData h = (ma - mi) / (nbins - 1);
    // Array<TData> boundaries = linspace(mi - h / 2, ma + h / 2, nbins + 1);
    hist = zeros<size_t>(ArrayShape{nbins});
    for(size_t i = 0; i < fdata.length(); ++i) {
        // for(size_t j = 1; j < boundaries.length(); ++j) {
        //     if (const TData& v = fdata(i); v >= boundaries(j - 1) && v < boundaries(j)) {
        //         ++hist(j - 1);
        //     }
        // }
        ++hist(((nbins - 1) * (fdata(i) - mi)) / (ma - mi));
    }
    bins = linspace(mi, ma, nbins);
}

}
