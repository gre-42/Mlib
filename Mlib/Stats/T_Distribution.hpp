#pragma once
#include <Mlib/Stats/Incomplete_Beta_Distribution.hpp>

namespace Mlib {

template <class TData>
TData student_t_cdf(TData t, TData v) {
    /*The cumulative distribution function (CDF) for Student's t distribution*/
    TData x = (t + std::sqrt(t * t + v)) / (2.0 * std::sqrt(t * t + v));
    TData prob = incbeta<TData>(v / 2, v / 2, x);
    return prob;
}

template <class TData>
TData student_t_sf(TData t, TData v) {
    return student_t_cdf(-t, v);
}

}
