#pragma once
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <iostream>

namespace Mlib {

template <class TData, class TAscGradient, class TDesGradient>
void gradient_ascent_descent(
    Array<TData> &asc_x,
    Array<TData> &des_x,
    const TAscGradient& asc_gradient,
    const TDesGradient& des_gradient,
    TData asc_lambda,
    TData des_lambda,
    size_t nsteps)
{
    for(size_t i = 0; i < nsteps; ++i) {
        asc_x += asc_lambda * asc_gradient(asc_x, des_x);
        des_x -= des_lambda * des_gradient(asc_x, des_x);
    }
}

}
