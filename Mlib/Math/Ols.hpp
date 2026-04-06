#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <cmath>

namespace Mlib {

template <class TData>
class Ols {
public:
    Ols(const Array<TData>& predictors, const Array<TData>& responses, TData alpha = 0)
    {
        assert(predictors.ndim() == 1);
        assert(all(predictors.shape() == responses.shape()));
        mean_p_ = mean(predictors);
        mean_r_ = mean(responses);
        const Array<TData> pm = predictors - mean_p_;
        const Array<TData> rm = responses - mean_r_;
        std_p_ = std::sqrt(dot0d(pm, pm)) * (1 + alpha) + alpha;
        const TData std_r = std::sqrt(dot0d(rm, rm)) * (1 + alpha) + alpha;
        if (predictors.length() == 1) {
            r_ = NAN;
            coeff_ = 0;
        } else {
            r_ = (dot0d(pm, rm)) / std_r / std_p_;
            r_ = std::max<TData>(std::min<TData>(r_, 1), -1);
            coeff_ = std_r * r_ / std_p_;
        }
        // lerr() << pm << " - " << rm << " " << alpha;
        // lerr() << alpha << " " << mean_r_ << " " << std_r << " " << r_ << " " << std_p_ << " " << mean_p_;
    }
    TData predicted(const TData& predictor) const {
        return mean_r_ + coeff_ * (predictor - mean_p_);
    }
    TData confidence(const TData& predictor) const {
        const TData dist = squared((predictor - mean_p_) / std_p_);
        return 1 / (1 + dist) * (1 / (1 - std::abs(r_)) - 1);
    }
    TData offset() const {
        return mean_r_ - coeff_ * mean_p_;
    }
    TData slope() const {
        return coeff_;
    }
private:
    TData mean_r_;
    TData mean_p_;
    TData r_;
    TData std_p_;
    TData coeff_;
};

}
