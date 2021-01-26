#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Sort.hpp>

namespace Mlib {

template <class TDataX>
class Quantiles {
public:
    explicit Quantiles(const Array<TDataX>& x)
    : sx_{sorted(x)}
    {
        if (x.length() == 0) {
            throw std::runtime_error("Cannot compute quantiles for an empty array");
        }
    }
    template <class TDataQ>
    const TDataX& operator () (const TDataQ& q, const TDataQ& offset = TDataQ(0.5)) {
        assert(q >= 0);
        assert(q <= 1);
        return sx_(size_t(q * (sx_.length() - 1) + offset));
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
