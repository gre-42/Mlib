#pragma once
#include <cmath>

namespace Mlib {

template <class TData, class TFloat>
class PidController {
public:
    PidController(const TFloat& p, const TFloat& i, const TFloat& d, const TFloat& a)
    : initialized_{ false },
      p_{ p },
      i_{ i },
      d_{ d },
      a_{ a },
      I_( 0 ),
      e_old_{ NAN }
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
    PidController changed_time_step(const TFloat& from, const TFloat& to) {
        // The factor f4 was determined by trial and error.
        auto f = from / to;
        auto f2 = f * f;
        auto f4 = f2 * f2;
        return {
            p_ * f4,
            i_ * f4,
            d_ * f4,
            std::pow(a_, f) };
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
