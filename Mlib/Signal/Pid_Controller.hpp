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
    TData operator () (const TData& e) {
        I_ = (1 - a_) * I_ + a_ * e;
        TData result = initialized_
            ? p_ * e + i_ * I_ + d_ * (e - e_old_)
            : p_ * e + i_ * I_;
        e_old_ = e;
        initialized_ = true;
        return result;
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
