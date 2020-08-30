#pragma once
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace Mlib {

template <class TData>
class Interp {
public:
    Interp(
        const std::vector<TData>& x,
        const std::vector<TData>& y,
        bool throw_out_of_range = true,
        const TData low = NAN,
        const TData high = NAN)
    : x_{x},
      y_{y},
      throw_out_of_range_{throw_out_of_range},
      low_{low},
      high_{high}
    {}
    TData operator () (TData vx) const {
        if (x_.size() != y_.size()) {
            throw std::runtime_error("size mismatch");
        }
        if (x_.size() < 1) {
            throw std::runtime_error("size must be >= 1");
        }
        if (vx < x_[0]) {
            if (throw_out_of_range_) {
                throw std::runtime_error("interpolation value too small");
            }
            return low_;
        }
        auto it = std::lower_bound(x_.begin(), x_.end(), vx);
        if (it == x_.end()) {
            if (throw_out_of_range_) {
                throw std::runtime_error("interpolation value too large");
            }
            return high_;
        }
        if (it == x_.begin()) {
            return y_[0];
        }
        size_t i = it - x_.begin();
        TData alpha = (vx  - x_[i - 1]) / (x_[i] - x_[i - 1]);
        return y_[i - 1] * (1 - alpha) + y_[i] * alpha;
    }
private:
    std::vector<TData> x_;
    std::vector<TData> y_;
    bool throw_out_of_range_;
    TData low_;
    TData high_;
};

}
