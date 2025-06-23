#pragma once
#include <Mlib/Uninitialized.hpp>
#include <cmath>
#include <optional>

namespace Mlib {

template <class TData, class TFloat=TData>
class ExponentialSmoother {
public:
    explicit ExponentialSmoother(const TFloat& alpha)
        : alpha_{ alpha }
    {}
    ExponentialSmoother(const TFloat& alpha, Uninitialized)
        : alpha_{ alpha }
    {}
    ExponentialSmoother(const TFloat& alpha, const TData& x0)
        : alpha_{ alpha }
        , s_{ x0 }
    {}
    const TData& operator () (const TData& x) {
        if (s_.has_value()) {
            *s_ = (1 - alpha_) * (*s_) + alpha_ * x;
        } else {
            s_ = x;
        }
        return *s_;
    }
    const TData& xhat() const {
        return s_.value();
    }
    void reset() {
        s_.reset();
    }
private:
    TFloat alpha_;
    std::optional<TData> s_;
};

}
