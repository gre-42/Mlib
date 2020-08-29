#pragma once
#include <cmath>

namespace Mlib {

template <class TData>
class ExponentialSmoother {
public:
    explicit ExponentialSmoother(const TData& alpha, const TData& x0 = NAN)
    : alpha_{alpha},
      s_{x0}
    {}
    TData operator () (const TData& x) {
        if (std::isnan(s_)) {
            s_ = x;
        } else {
            s_ = (1 - alpha_) * s_ + alpha_ * x;
        }
        return s_;
    }
private:
    TData alpha_;
    TData s_;
};

}
