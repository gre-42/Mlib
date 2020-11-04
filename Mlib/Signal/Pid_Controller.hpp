#pragma once

namespace Mlib {

template <class TData, class TFloat>
class PidController {
public:
    PidController(const TFloat& p, const TFloat& i, const TFloat& d, const TFloat& a)
    : initialized_{false},
      p_{p},
      i_{i},
      d_{d},
      a_{a},
      I_(0)
    {}
    const TData& operator () (const TData& e) {
        I_ = (1 - a_) * I_ + a_ * e;
        if (!initialized_) {
            e_old_ = e;
            initialized_ = true;
        } else {
            e_old_ = p_ * e + i_ * I_ + d_ * (e - e_old_);
        }
        return e_old_;
    }
private:
    bool initialized_;
    TFloat p_;
    TFloat i_;
    TFloat d_;
    TFloat a_;
    TData I_;
    TData e_old_;
};

}
