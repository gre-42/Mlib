#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Stats/RansacOptions.hpp>
#include <Mlib/Stats/Sort.hpp>
#include <algorithm>
#include <limits>
#include <random>

namespace Mlib {

/**
 * Random sampling consensus (RANSAC) algorithm
 *
 * The algorithm can be disabled as follows.
 * - ro.inlier_distance_thresh = infinity
 * - ro.nelems_small >= nelems_large
 * - ro.ncalls = 1
 */
template <class TData, class TCallable>
Array<size_t> ransac(
    size_t nelems_large,
    const RansacOptions<TData>& ro,
    const TCallable& callable)
{
    assert(ro.nelems_small > 0);
    assert(
        (nelems_large >= ro.nelems_small) ||
        ro.inlier_distance_thresh == std::numeric_limits<TData>::infinity());
    Array<size_t> ids_large = arange<size_t>(nelems_large);
    std::default_random_engine g(ro.seed);
    TData best_mean_residual = std::numeric_limits<TData>::infinity();
    Array<size_t> best_indices;

    for (size_t i = 0; i < ro.ncalls; ++i) {

        std::shuffle(&ids_large(0), &ids_large(ids_large.length() - 1), g);
        Array<size_t> perm(&ids_large(0), &ids_large(0) + std::min(ro.nelems_small, nelems_large));
        sort(perm);
        Array<TData> positive_residual = substitute_nans(callable(perm), TData(INFINITY));
        if (positive_residual.length() != nelems_large) {
            throw std::runtime_error(
                "Residual length (" +
                std::to_string(positive_residual.length())  +
                ")  does not match nelems_large (" +
                std::to_string(nelems_large) + ")");
        }
        Array<bool> also_inliers = (positive_residual <= ro.inlier_distance_thresh);

        if (count_nonzero(also_inliers) > ro.inlier_count_thresh) {
            Array<size_t> better_indices = arange<size_t>(nelems_large)[also_inliers];
            Array<TData> better_positive_residual = substitute_nans(callable(better_indices), TData(INFINITY));
            TData better_mean_residual = mean(better_positive_residual);

            // std::cerr << "---- " << better_mean_residual << " " << best_mean_residual << std::endl;
            if (better_mean_residual < best_mean_residual) {
                best_indices.destroy();
                best_indices = better_indices;
                best_mean_residual = better_mean_residual;
            }
        }
    }
    return best_indices;
}

}
