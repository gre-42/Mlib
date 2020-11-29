#pragma once
#include <cstddef>
#include <stdexcept>
#include <string>

namespace Mlib {

template <class TFloat, class TFunction, class TDerivative>
TFloat newton_1d(
    const TFunction& function,
    const TDerivative& derivative,
    TFloat x,
    TFloat eps = 1e-7,
    size_t niterations = 10,
    float werror = true)
{
    for(size_t i = 0; i < niterations; ++i) {
        TFloat dx = -function(x) / derivative(x);
        if (std::abs(dx) < eps) {
            return x;
        }
        x += dx;
    }
    if (werror) {
        throw std::runtime_error("Newton did not converge after " + std::to_string(niterations) + " steps");
    } else {
        return NAN;
    }
}

}
