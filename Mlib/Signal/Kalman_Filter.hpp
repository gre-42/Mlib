#pragma once
#include <cmath>

namespace Mlib {

template <class TData>
class KalmanFilter {
public:
    /**
     * Kalman filter

     * Parameters
     * ----------
     * ziter : iterable
     *     Observations ~ 𝓝(x, √R).
     * xhat : TData, optional
     *     Initial estimate for x.
     * Q : TData
     *     Process variance.
     * R : TData
     *     Measurement variance.
     * P : TData
     *     Posteriori error variance.
     *
     * References
     * ----------
     *     http://wiki.scipy.org/Cookbook/KalmanFiltering
     */
    KalmanFilter(const TData& Q=1e-5, const TData& R=1e-2, const TData& P=1, const TData& xhat=NAN)
    : Q_{Q},
      R_{R},
      P_{P},
      xhat_{xhat}
    {}
    TData operator () (TData z) {
        if (std::isnan(xhat_)) {
            xhat_ = z;  // a posteri estimate of x
        } else {
            // time update
            TData xhatminus = xhat_;  // a priori estimate of x
            TData Pminus = P_ + Q_;   // a priori error estimate

            // measurement update
            TData K = Pminus / (Pminus + R_);  // gain or blending factor
            xhat_ = xhatminus + K * (z - xhatminus);
            P_ = (1 - K) * Pminus;
        }
        return xhat_;
    }
private:
    TData Q_;
    TData R_;
    TData P_;
    TData xhat_;
};

}
