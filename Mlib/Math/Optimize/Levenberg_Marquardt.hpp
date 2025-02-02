#pragma once
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Generic_Optimization.hpp>
#include <optional>

namespace Mlib {

template <class TData, class TdAT_A = Array<TData>, class TdAT_b = Array<TData>, class TX, class TF, class TJacobian>
TX levenberg_marquardt(
    const TX& x0,
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
    const TdAT_A* dAT_A = nullptr,
    const TdAT_b* dAT_b = nullptr)
{
    return generic_optimization(
        x0,
        [&](const TX& x, size_t i){
            return y - f(x);
        },
        [](const TX& x, const Array<TData>& residual, size_t i){
            return (residual.length() == 0)
                ? 0
                : sum(squared(residual)) / residual.length();
        },
        [&](const TX& x, const Array<TData>& residual, size_t i){
            std::optional<TX> dx;
            lstsq_chol_1d(
                dx,
                J(x),
                residual,
                i < nburnin ? alpha : alpha2,
                i < nburnin ? beta : beta2,
                dAT_A,
                dAT_b);
            return x + dx.value();
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
