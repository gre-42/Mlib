#pragma once
#include <Mlib/Geometry/Intersection/Interval.hpp>
#include <Mlib/Math/Lerp.hpp>
#include <Mlib/Uninitialized.hpp>
#include <algorithm>
#include <cmath>
#include <optional>

namespace Mlib {

template <class TData, class TFloat=TData>
class ClampedExponentialSmoother {
public:
    explicit ClampedExponentialSmoother(const TFloat& alpha, const Interval<TData>& interval)
        : alpha_{ alpha }
        , interval_{ interval }
    {}
    ClampedExponentialSmoother(const TFloat& alpha, const Interval<TData>& interval, Uninitialized)
        : alpha_{ alpha }
        , interval_{ interval }
    {}
    template <class TX0>
    ClampedExponentialSmoother(const TFloat& alpha, const Interval<TData>& interval, const TX0& x0)
        : alpha_{ alpha }
        , interval_{ interval }
        , s_{ x0 }
    {}
    const TData& operator () (const TData& x) {
        if (s_.has_value()) {
            *s_ += std::clamp(lerp(*s_, x, alpha_) - *s_, interval_.min, interval_.max);
        } else {
            s_ = x;
        }
        return *s_;
    }
    ClampedExponentialSmoother changed_time_step(const TFloat& from, const TFloat& to) const {
        auto f = to / from;
        return { std::pow(alpha_, 1 / f), {interval_.min * f, interval_.max * f}, s_ };
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
    Interval<TData> interval_; 
    std::optional<TData> s_;
};

}
