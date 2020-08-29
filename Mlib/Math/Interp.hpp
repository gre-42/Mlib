#pragma once
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
        if (x_.size() < 2) {
            throw std::runtime_error("size must be >= 2");
        }
        if (vx < x_[0]) {
            if (throw_out_of_range_) {
                throw std::runtime_error("interpolation value too small");
            }
            return low_;
        }
        if (vx > x_[x_.size() - 1]) {
            if (throw_out_of_range_) {
                throw std::runtime_error("interpolation value too large");
            }
            return high_;
        }
        for(size_t i = 0; i < x_.size() - 1; ++i) {
            if (vx <= x_[i + 1]) {
                if (vx < x_[i]) {
                    throw std::runtime_error("interval error: " + std::to_string(x_[i]) + " " + std::to_string(x_[i + 1]) + " " + std::to_string(vx));
                }
                TData alpha = (vx  - x_[i]) / (x_[i + 1] - x_[i]);
                return y_[i] * (1 - alpha) + y_[i + 1] * alpha;
            }
        }
        throw std::runtime_error("interp did not find an internal for " + std::to_string(vx));
    }
private:
    std::vector<TData> x_;
    std::vector<TData> y_;
    bool throw_out_of_range_;
    TData low_;
    TData high_;
};

}
