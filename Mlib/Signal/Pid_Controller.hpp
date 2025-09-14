#pragma once
#include <Mlib/Math/Lerp.hpp>
#include <cmath>
#include <optional>
#include <ostream>

namespace Mlib {

template <class TData, class TFloat>
class PidController {
    template <class TData2, class TFloat2>
    friend std::ostream& operator << (std::ostream& ostr, const PidController<TData2, TFloat2>& pid);
public:
    PidController(
        const TFloat& p,
        const TFloat& i,
        const TFloat& d,
        const TFloat& a,
        const TData& I = (TData)0,
        std::optional<TData> e_old = std::nullopt)
        : p_{ p }
        , i_{ i }
        , d_{ d }
        , a_{ a }
        , I_( I )
        , e_old_{ e_old }
    {}
    TData operator () (const TData& e) {
        I_ = lerp(I_, e, a_);
        TData result = e_old_.has_value()
            ? p_ * e + i_ * I_ + d_ * (e - *e_old_)
            : p_ * e + i_ * I_;
        e_old_ = e;
        return result;
    }
    TData operator () (const TData& e, const TFloat& from, const TFloat& to) {
        auto cts = changed_time_step(from, to);
        auto result = cts(e);
        I_ = cts.I_;
        e_old_ = e;
        return result;
    }
    PidController changed_time_step(const TFloat& from, const TFloat& to) {
        // The factors were determined by trial and error in the test "test_pid".
        auto f = from / to;
        return {
            p_,
            i_,
            d_ * f,
            std::pow(a_, f),
            I_,
            e_old_ };
    }
private:
    TFloat p_;
    TFloat i_;
    TFloat d_;
    TFloat a_;
    TData I_;
    std::optional<TData> e_old_;
};

template <class TData, class TFloat>
inline std::ostream& operator << (std::ostream& ostr, const PidController<TData, TFloat>& pid) {
    ostr <<
        "p=" << pid.p_ <<
        " i=" << pid.i_ <<
        " d=" << pid.d_ <<
        " a=" << pid.a_ <<
        " I=" << pid.I_;
    if (pid.e_old_.has_value()) {
        ostr << " e_old=" << *pid.e_old_ ;
    } else {
        ostr << " e_old=null";
    }
    return ostr;
}

}
