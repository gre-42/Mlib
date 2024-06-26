#pragma once
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <iostream>

namespace Mlib {

template <class TData, class TFunction, class TGradient>
Array<TData> gradient_descent(
    const Array<TData>& x0,
    const TFunction& func,
    const TGradient& gradient,
    size_t nsteps,
    bool line_search_v1 = false)
{
    Array<TData> x = x0.copy();
    Array<TData> dx;
    TData step_size = 1.f;
    for (size_t i = 0; i < nsteps; ++i) {
        TData f0 = func(x);
        dx = gradient(x);
        // TData len = std::sqrt(sum(squared(dx)));
        // if (len < 1e-12) {
        //     lerr() << "Gradient is zero, stopping after " << i << " steps (f=" << f0 << ")" << sum(squared(x));
        //     return x;
        // }
        // dx /= len;
        step_size *= 2;
        if (line_search_v1) {
            while(!(func(x - step_size * dx) < f0)) {
                // lerr() << "!(" << func(x - step_size * dx) << " < " << f0 << ")";
                step_size /= 2;
                if (step_size < 1e-12) {
                    lerr() <<
                        "Negative gradient does not descent, stopping after " <<
                        i << " steps";
                    return x;
                }
            }
        } else {
            float best_step_size = NAN;
            float best_step_value = NAN;
            while(step_size > 1e-12) {
                TData step_value = func(x - step_size * dx);
                if (std::isnan(best_step_value) || (step_value < best_step_value)) {
                    best_step_size = step_size;
                    best_step_value = step_value;
                } else if (best_step_value < f0) {
                    break;
                }
                step_size /= 2;
            }
            if (!(best_step_value < f0)) {
                // lerr() << "!(" << func(x - step_size * dx) << " < " << f0 << ")";
                lerr() <<
                    "Negative gradient does not descent, stopping after " <<
                    i << " steps";
                return x;
            }
            step_size = best_step_size;
        }
        // lerr() << "step[" << i << "]: " << step_size;
        x -= step_size * dx;
    }
    return x;
}

}
