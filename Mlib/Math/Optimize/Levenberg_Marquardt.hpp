#pragma once
#include <Mlib/Math/Optimize/Generic_Optimization.hpp>

namespace Mlib {

template <class TData, class TF, class TJacobian>
Array<TData> levenberg_marquardt(
    const Array<TData>& x0,
    const Array<TData>& y,
    const TF& f,
    const TJacobian& J,
    const TData& alpha,
    const TData& beta,
    const TData& alpha2,
    const TData& beta2,
    const TData& min_redux,
    size_t niterations = 100,
    size_t nburnin = 5,
    size_t nmisses = 3,
    bool print_residual = true,
    bool nothrow = false,
    Array<TData>* final_residual = nullptr,
    const TData* max_residual = nullptr,
    const Array<TData>* dAT_A = nullptr,
    const Array<TData>* dAT_b = nullptr)
{
    return generic_optimization(
        x0,
        [&](const Array<TData>& x, size_t i){
            return y - f(x);
        },
        [](const Array<TData>& x, const Array<TData>& residual, size_t i){
            return (residual.length() == 0)
                ? 0
                : sum(squared(residual)) / residual.length();
        },
        [&](const Array<TData>& x, const Array<TData>& residual, size_t i){
            return x + lstsq_chol_1d(
                J(x),
                residual,
                i < nburnin ? alpha : alpha2,
                i < nburnin ? beta : beta2,
                dAT_A,
                dAT_b);
        },
        min_redux,
        niterations,
        nmisses,
        print_residual,
        nothrow,
        final_residual,
        max_residual);
}

}
