#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>

namespace Mlib {

template <class TData, class TX, class TGetResidual, class TGetResidualNorm, class TGetNextX>
TX generic_optimization(
    const TX& x0,
    const TGetResidual& get_residual,
    const TGetResidualNorm& get_residual_norm,
    const TGetNextX& get_next_x,
    const TData& min_redux,
    size_t niterations = 100,
    size_t nmisses = 3,
    bool print_residual = true,
    bool nothrow = false,
    Array<TData>* final_residual = nullptr,
    const TData* max_residual = nullptr)
{
    assert(nmisses > 0);
    // f(x0) + J * d = 0
    // -J * d = f(x0)
    // -J^T * J * d = J^T * f(x0)
    TX x = uninitialized;
    TX x_best = uninitialized;
    x = x0;
    x_best = x0;
    TData old_ssq_residual = std::numeric_limits<TData>::infinity();
    Array<TData> residual;
    size_t i;
    for (i = 0; i < niterations; ++i) {
        // lerr() << "i " << i;
        // lerr() << "J " << J(x).shape();
        // lerr() << "f " << f(x).shape();
        residual = get_residual(x, i);
        if (max_residual != nullptr) {
            if (final_residual != nullptr) {
                *final_residual = residual;
            }
            for (size_t r = 0; r < residual.length(); ++r) {
                if (std::abs(residual(r)) > *max_residual) {
                    residual(r) = sign(residual(r)) * (*max_residual);
                }
            }
        }
        TData ssq_residual = get_residual_norm(x, residual, i);
        // if (dAT_A != nullptr) {
        //     ssq_residual += dot(dot(x, *dAT_A), x)();
        // }
        // if (dAT_b != nullptr) {
        //     ssq_residual += 2 * dot(x, *dAT_b)();
        // }
        TData redux = (old_ssq_residual == std::numeric_limits<TData>::infinity()
            ? NAN
            : (old_ssq_residual - ssq_residual) / old_ssq_residual);
        if (print_residual) {
            lerr() << "residual " << ssq_residual << " redux " << redux;
        }
        if (ssq_residual >= old_ssq_residual) {
            if (--nmisses == 0) {
                break;
            }
        } else {
            x_best = x;
        }
        if (!std::isnan(redux) && (redux < min_redux)) {
            break;
        }
        old_ssq_residual = ssq_residual;
        x = get_next_x(x, residual, i);
    }
    if ((max_residual == nullptr) && (final_residual != nullptr)) {
        *final_residual = residual;
    }
    if ((i < niterations) || nothrow) {
        return x_best;
    } else {
        THROW_OR_ABORT("levenberg_marquardt did not converge");
    }
}

}
