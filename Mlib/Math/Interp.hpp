#pragma once
#include <algorithm>
#include <iosfwd>
#include <stdexcept>
#include <vector>

namespace Mlib {

enum class OutOfRangeBehavior {
    THROW,
    EXPLICIT,
    CLAMP
};

template <class TData>
class Interp;

template <class TData>
std::ostream& operator << (std::ostream& ostr, const Interp<TData>& interp);

template <class TData>
class Interp {
    friend std::ostream& operator << <TData>(std::ostream& ostr, const Interp<TData>& interp);
public:
    Interp(
        const std::vector<TData>& x,
        const std::vector<TData>& y,
        OutOfRangeBehavior out_of_range_behavior = OutOfRangeBehavior::THROW,
        const TData low = NAN,
        const TData high = NAN)
    : x_{x},
      y_{y},
      out_of_range_behavior_{out_of_range_behavior},
      low_{low},
      high_{high}
    {
        if (x_.size() != y_.size()) {
            throw std::runtime_error("size mismatch");
        }
    }
    TData operator () (const TData& vx) const {
        if (x_.size() < 1) {
            throw std::runtime_error("size must be >= 1");
        }
        if (vx < x_[0]) {
            switch (out_of_range_behavior_) {
            case OutOfRangeBehavior::THROW:
                throw std::runtime_error("interpolation value too small");
            case OutOfRangeBehavior::EXPLICIT:
                return low_;
            case OutOfRangeBehavior::CLAMP:
                return y_.front();
            default:
                throw std::runtime_error("Unknown interpolation behavior");
            }
        }
        auto it = std::lower_bound(x_.begin(), x_.end(), vx);
        if (it == x_.end()) {
            switch (out_of_range_behavior_) {
            case OutOfRangeBehavior::THROW:
                throw std::runtime_error("interpolation value too large");
            case OutOfRangeBehavior::EXPLICIT:
                return high_;
            case OutOfRangeBehavior::CLAMP:
                return y_.back();
            default:
                throw std::runtime_error("Unknown interpolation behavior");
            }
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
    OutOfRangeBehavior out_of_range_behavior_;
    TData low_;
    TData high_;
};

template <class TData>
std::ostream& operator << (std::ostream& ostr, const Interp<TData>& interp) {
    for(size_t i = 0; i < interp.x_.size(); ++i) {
        ostr << interp.x_[i] << " -> " << interp.y_[i] << std::endl;
    }
    return ostr;
}

}
