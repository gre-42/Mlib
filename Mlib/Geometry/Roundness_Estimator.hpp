#pragma once
#include <Mlib/Math/Fixed_Determinant.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Trace.hpp>

namespace Mlib {

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
