#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

/*
 * Power iteration on a symmetric matrix.
 * Eigenvector is orthogonalized w.r.t. orthonormal v[:i].
 * Eigenvalue is stored in s.
 */
template <class TData>
void power_iteration(
    const Array<TData>& a,
    Array<TData>& uT,
    typename FloatType<TData>::value_type& s,
    size_t i,
    size_t seed = 1)
{
    assert(a.ndim() == 2);
    assert(a.shape(0) == a.shape(1));
    randomize_array(uT[i], seed);

    for (size_t n = 0; n < 30 * a.shape(0); n++) {
        for (size_t r = 0; r < i; ++r) {
            uT[i] -= uT[r] * outer(uT[i], uT[r])();
        }
        Array<TData> ui_old;
        ui_old = uT[i];
        uT[i] = outer(uT[i], a);
        typename FloatType<TData>::value_type s_old = s;
        s = std::sqrt(sum(norm(uT[i])));
        if (s < 1e-12) {
            // undo, and abort, because diff will be 0
            uT[i] = ui_old;
        } else {
            uT[i] /= s;
        }

        {
            Array<TData> diff = uT[i] - ui_old;
            auto diff_norm = sum(norm(diff));
            if (diff_norm < 1e-12) {
                return;
            }
        }

        {
            Array<TData> diff = uT[i] + ui_old;
            auto diff_norm = sum(norm(diff));
            if (diff_norm < 1e-12) {
                s = -s;
                return;
            }
        }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        // Required for close-to-identical eigenvalues.
        if ((n > 0) && (std::abs(s - s_old) < 1e-7)) {
            return;
        }
#pragma GCC diagnostic pop
    }
    throw PowerIterationDidNotConvergeError("power iteration did not converge");
}

}
