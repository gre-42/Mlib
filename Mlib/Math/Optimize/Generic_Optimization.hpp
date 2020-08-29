#pragma once
#include <Mlib/Array/Array.hpp>
#include <iostream>

namespace Mlib {

template <class TData, class TGetResidual, class TGetResidualNorm, class TGetNextX>
Array<TData> generic_optimization(
    const Array<TData>& x0,
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
    Array<TData> x;
    x = x0;
    TData old_ssq_residual = std::numeric_limits<TData>::infinity();
    Array<TData> residual;
    for(size_t i = 0; i < niterations; ++i) {
        // std::cerr << "i " << i << std::endl;
        // std::cerr << "J " << J(x).shape() << std::endl;
        // std::cerr << "f " << f(x).shape() << std::endl;
        residual = get_residual(x, i);
        if (max_residual != nullptr) {
            if (final_residual != nullptr) {
                *final_residual = residual;
            }
            for(size_t r = 0; r < residual.length(); ++r) {
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
            std::cerr << "residual " << ssq_residual << " redux " << redux << std::endl;
        }
        if (ssq_residual >= old_ssq_residual) {
            if (--nmisses == 0) {
                return x;
            }
        }
        if (!std::isnan(redux) && (redux < min_redux)) {
            return x;
        }
        old_ssq_residual = ssq_residual;
        x = get_next_x(x, residual, i);
    }
    if ((max_residual == nullptr) && (final_residual != nullptr)) {
        *final_residual = residual;
    }
    if (nothrow) {
        return x;
    } else {
        throw std::runtime_error("levenberg_marquardt did not converge");
    }
}

}
