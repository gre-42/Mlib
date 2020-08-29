#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Sort.hpp>

namespace Mlib {

template <class TDataX, class TDataQ>
Array<TDataX> quantiles(const Array<TDataX>& x, const Array<TDataQ>& q) {
    if (x.length() == 0) {
        throw std::runtime_error("Cannot compute quantiles for an empty array");
    }
    Array<TDataX> sx = sorted(x);
    Array<TDataX> res(q.shape());
    for(size_t i = 0; i < q.length(); ++i) {
        assert(q(i) >= 0);
        assert(q(i) <= 1);
        res(i) = sx(size_t(q(i) * (x.length() - 1) + TDataQ(0.5)));
    }
    return res;
}

template <class TDataX, class TDataQ>
TDataX quantile(const Array<TDataX>& x, const TDataQ& q) {
    return quantiles(x, Array<TDataQ>{q})(0);
}

template <class TDataX, class TDataQ>
Array<TDataX> nanquantiles(const Array<TDataX>& x, const Array<TDataQ>& q) {
    return quantiles(x[!isnan(x)], q);
}

template <class TDataX, class TDataQ>
TDataX nanquantile(const Array<TDataX>& x, const TDataQ& q) {
    return quantile(x[!isnan(x)], q);
}

}
