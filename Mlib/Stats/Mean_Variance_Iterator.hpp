#pragma once
#include <Mlib/Stats/T_Distribution.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

/**
 * Iterator yielding mean and variance.
 * http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
 */
template <class TData>
class MeanVarianceIterator{
public:
    void add(const TData& x) {
        ++n_;
        TData delta = x - mean_;
        mean_ += delta / (TData)n_;
        M2_ += delta * (x - mean_);
    }
    TData mean() const {
        return mean_;
    }
    TData var() const {
        if (n_ < 2) {
            THROW_OR_ABORT("n < 2, can not compute variance");
        }
        return M2_ / TData(n_ - 1);
    }
    TData cohens_d(const TData& epsilon = 0) const {
        return mean() / (std::sqrt(var()) + epsilon);
    }
    TData t(const TData& epsilon = 0) const {
        return mean() / (std::sqrt(var() / TData(n_)) + epsilon);
    }
    TData p1() const {
        if (var() < 1e-12) {
            return 0;
        } else {
            return student_t_sf<TData>(t(), TData(n_ - 1));
        }
    }
    TData p2() const {
        if (var() < 1e-12) {
            return 0;
        } else {
            return 2 * student_t_sf<TData>(std::abs(t()), TData(n_ - 1));
        }
    }
    size_t n() const {
        return n_;
    }
private:
    size_t n_ = 0;
    TData mean_ = 0;
    TData M2_ = 0;
};

}
