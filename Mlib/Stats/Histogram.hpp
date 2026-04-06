#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <cmath>
#include <stdexcept>

namespace Mlib {

enum class OutOfBoundsBehavior {
    CLAMP,
    THROW
};

template <class TData>
class Histogram {
public:
    Histogram(const Array<TData>& data, size_t nbins = 10) {
        if (nbins <= 1) {
            throw std::runtime_error("Number of bins must be > 1");
        }
        Array<TData> fdata = data[Mlib::isfinite(data)];
        mi_ = min(fdata);
        ma_ = max(fdata);
        nobservations = fdata.length();
        if ((ma_ - mi_) < 1e-12) {
            nbins = 1;
        }
        // TData h = (ma - mi) / (nbins - 1);
        // Array<TData> boundaries = linspace(mi - h / 2, ma + h / 2, nbins + 1);
        hist_ = zeros<size_t>(ArrayShape{nbins});
        for (const TData& v : fdata.flat_iterable()) {
            // for (size_t j = 1; j < boundaries.length(); ++j) {
            //     if (const TData& v = fdata(i); v >= boundaries(j - 1) && v < boundaries(j)) {
            //         ++hist(j - 1);
            //     }
            // }
            ++hist_(bin_id(v, OutOfBoundsBehavior::THROW));
        }
    }
    const Array<size_t>& hist() const {
        return hist_;
    }
    Array<TData> bin_boundaries() const {
        return linspace(mi_, ma_, hist_.length() + 1);
    }
    Array<TData> bin_centers() const {
        auto dx = TData((ma_ - mi_) / double(hist_.length() * 2));
        return linspace(TData(mi_ + dx), TData(ma_ - dx), hist_.length());
    }
    size_t bin_id(
        const TData& data,
        OutOfBoundsBehavior out_of_bounds_behavior) const
    {
        if (hist_.length() == 1) {
            return 0;
        }
        auto fbin = std::floor((TData(hist_.length() - 1) * (data - mi_)) / (ma_ - mi_));
        if (fbin < 0) {
            if (out_of_bounds_behavior == OutOfBoundsBehavior::CLAMP) {
                return 0;
            }
            throw std::runtime_error("Bin index too low");
        }
        auto bin = float_to_integral<size_t>(fbin);
        if (bin >= hist_.length()) {
            if (out_of_bounds_behavior == OutOfBoundsBehavior::CLAMP) {
                return hist_.length() - 1;
            }
            throw std::runtime_error("Bin index too high");
        }
        return bin;
    }
    size_t nbins() const {
        return hist_.length();
    }
    size_t nobservations;
private:
    Array<size_t> hist_;
    TData mi_;
    TData ma_;
};

template <class TData>
void histogram(const Array<TData>& data, Array<size_t>& hist, Array<TData>& bins, size_t nbins = 10) {
    Histogram hm{data, nbins};
    hist = hm.hist();
    bins = hm.bin_centers();
}

}
