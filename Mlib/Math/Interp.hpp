#pragma once
#include "Interp_Fwd.hpp"
#include <algorithm>
#include <cmath>
#include <iosfwd>
#include <stdexcept>
#include <vector>

namespace Mlib {

enum class OutOfRangeBehavior {
    THROW,
    EXPLICIT,
    CLAMP
};

template <class TDataX, class TDataY>
std::ostream& operator << (std::ostream& ostr, const Interp<TDataX, TDataY>& interp);

template <class TDataX, class TDataY>
class Interp {
    friend std::ostream& operator << <TDataX, TDataY>(std::ostream& ostr, const Interp<TDataX, TDataY>& interp);
public:
    Interp(
        const std::vector<TDataX>& x,
        const std::vector<TDataY>& y,
        OutOfRangeBehavior out_of_range_behavior = OutOfRangeBehavior::THROW,
        const TDataY low = TDataY(NAN),
        const TDataY high = TDataY(NAN))
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
    TDataY operator () (const TDataX& vx) const {
        if (x_.empty()) {
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
        TDataX alpha = (vx  - x_[i - 1]) / (x_[i] - x_[i - 1]);
        return y_[i - 1] * (1 - alpha) + y_[i] * alpha;
    }
    bool is_within_range(const TDataX& vx) const {
        if (x_.empty()) {
            return false;
        }
        return (vx >= x_[0]) && (vx <= x_[x_.size() - 1]);
    }
private:
    std::vector<TDataX> x_;
    std::vector<TDataY> y_;
    OutOfRangeBehavior out_of_range_behavior_;
    TDataY low_;
    TDataY high_;
};

template <class TDataX, class TDataY>
std::ostream& operator << (std::ostream& ostr, const Interp<TDataX, TDataY>& interp) {
    for (size_t i = 0; i < interp.x_.size(); ++i) {
        ostr << interp.x_[i] << " -> " << interp.y_[i] << '\n';
    }
    return ostr;
}

}
