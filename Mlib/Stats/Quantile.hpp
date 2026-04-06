#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>
#include <Mlib/Stats/Sort.hpp>
#include <stdexcept>

namespace Mlib {

template <class TDataX>
class Quantiles {
public:
    explicit Quantiles(const Array<TDataX>& x)
        : sx_(sorted(x))
    {
        if (sx_.length() == 0) {
            throw std::runtime_error("Cannot compute quantiles for an empty array");
        }
    }
    template <class TDataQ>
    const TDataX& operator () (const TDataQ& q) const {
        auto findex = std::floor(q * TDataQ(sx_.length() - 1));
        if (findex < 0) {
            throw std::runtime_error("Quantile too low");
        }
        auto index = float_to_integral<size_t>(findex);
        if (index >= sx_.length()) {
            throw std::runtime_error("Quantile too large");
        }
        return sx_(index);
    }
private:
    Array<TDataX> sx_;
};

template <class TDataX, class TDataQ>
Array<TDataX> quantiles(const Array<TDataX>& x, const Array<TDataQ>& q) {
    Quantiles<TDataX> sx{x};
    Array<TDataX> res(q.shape());
    for (size_t i = 0; i < q.length(); ++i) {
        res(i) = sx(q(i));
    }
    return res;
}

template <class TDataX, class TDataQ>
TDataX quantile(const Array<TDataX>& x, const TDataQ& q) {
    return quantiles(x, Array<TDataQ>{q})(0);
}

template <class TDataX, class TDataQ>
Array<TDataX> nanquantiles(const Array<TDataX>& x, const Array<TDataQ>& q) {
    return quantiles(x[!Mlib::isnan(x)], q);
}

template <class TDataX, class TDataQ>
TDataX nanquantile(const Array<TDataX>& x, const TDataQ& q) {
    return quantile(x[!Mlib::isnan(x)], q);
}

}
