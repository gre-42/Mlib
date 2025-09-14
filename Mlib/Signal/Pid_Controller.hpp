#pragma once
#include <Mlib/Signal/Exponential_Smoother.hpp>
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
        const std::optional<TData>& e_old = std::nullopt)
        : p_{ p }
        , i_{ i }
        , d_{ d }
        , I_{ a, I }
        , e_old_{ e_old }
    {}
    PidController(
        const TFloat& p,
        const TFloat& i,
        const TFloat& d,
        const ExponentialSmoother<TData, TFloat>& I,
        const std::optional<TData>& e_old)
        : p_{ p }
        , i_{ i }
        , d_{ d }
        , I_{ I }
        , e_old_{ e_old }
    {}
    TData operator () (const TData& e) {
        TData result = e_old_.has_value()
            ? p_ * e + i_ * I_(e) + d_ * (e - *e_old_)
            : p_ * e + i_ * I_(e);
        e_old_ = e;
        return result;
    }
    TData operator () (const TData& e, const TFloat& from, const TFloat& to) {
        auto cts = changed_time_step(from, to);
        auto result = cts(e);
        I_.set(*cts.I_.xhat());
        e_old_ = e;
        return result;
    }
    PidController changed_time_step(const TFloat& from, const TFloat& to) const {
        // The factors were determined by trial and error in the test "test_pid".
        auto f = from / to;
        return {
            p_,
            i_,
            d_ * f,
            I_.changed_time_step(from, to),
            e_old_ };
    }
private:
    TFloat p_;
    TFloat i_;
    TFloat d_;
    ExponentialSmoother<TData, TFloat> I_;
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
