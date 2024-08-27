#pragma once
#include <Mlib/Uninitialized.hpp>
#include <cmath>

namespace Mlib {

template <class TData, class TFloat=TData>
class ExponentialSmoother {
public:
    explicit ExponentialSmoother(const TFloat& alpha)
        : alpha_{ alpha }
        , s_is_initialized_{ false }
    {}
    ExponentialSmoother(const TFloat& alpha, Uninitialized)
        : alpha_{ alpha }
        , s_{ uninitialized }
        , s_is_initialized_{ false }
    {}
    ExponentialSmoother(const TFloat& alpha, const TData& x0)
        : alpha_{ alpha }
        , s_{ x0 }
        , s_is_initialized_{ true }
    {}
    const TData& operator () (const TData& x) {
        if (!s_is_initialized_) {
            s_ = x;
            s_is_initialized_ = true;
        } else {
            s_ = (1 - alpha_) * s_ + alpha_ * x;
        }
        return s_;
    }
    const TData& xhat() const {
        return s_;
    }
    void reset() {
        s_is_initialized_ = false;
    }
private:
    TFloat alpha_;
    TData s_;
    bool s_is_initialized_;
};

}
