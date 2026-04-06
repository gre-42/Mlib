#pragma once
#include <Mlib/Initialization/Uninitialized.hpp>
#include <Mlib/Math/Lerp.hpp>
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
    template <class TX0>
    ExponentialSmoother(const TFloat& alpha, const TX0& x0, const TFloat& dt)
        : alpha_{ std::pow(alpha, dt) }
        , s_{ x0 }
    {}
    const TData& operator () (const TData& x) {
        if (s_.has_value()) {
            *s_ = lerp(*s_, x, alpha_);
        } else {
            s_ = x;
        }
        return *s_;
    }
    const TData& operator () (const TData& x, const TFloat& dt) {
        if (s_.has_value()) {
            *s_ = lerp(*s_, x, std::pow(alpha_, 1 / dt));
        } else {
            s_ = x;
        }
        return *s_;
    }
    ExponentialSmoother changed_time_step(const TFloat& from, const TFloat& to) const {
        auto f = from / to;
        return { alpha_, s_, f };
    }
    const std::optional<TData>& xhat() const {
        return s_;
    }
    void set(const TData& xhat) {
        s_ = xhat;
    }
    void reset() {
        s_.reset();
    }
private:
    TFloat alpha_;
    std::optional<TData> s_;
};

}
