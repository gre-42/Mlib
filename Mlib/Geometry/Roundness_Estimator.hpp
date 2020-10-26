#pragma once
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <class TData>
inline TData det2x2(const FixedArray<TData, 2, 2>& a)
{
    return a(0, 0) * a(1, 1) - a(0, 1) * a(1, 0);
}

template <class TData>
inline TData trace2x2(const FixedArray<TData, 2, 2>& a)
{
    return a(0, 0) + a(1, 1);
}

class RoundnessEstimator {
public:
    RoundnessEstimator()
    : m_(0)
    {}
    void add_direction(const FixedArray<float, 2>& v) {
        m_ += dot2d(v.as_column_vector(), v.as_row_vector());
    }
    float roundness() const {
        return det2x2(m_) / squared(trace2x2(m_)) * 4;
    }
private:
    FixedArray<float, 2, 2> m_;
};

}
