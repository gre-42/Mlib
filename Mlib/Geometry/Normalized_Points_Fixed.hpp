#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

enum class ScaleMode {
    NONE,
    PRESERVE_ASPECT_RATIO,
    DIAGONAL
};

enum class OffsetMode {
    CENTERED,
    MINIMUM
};

class NormalizedPointsFixed {
public:
    NormalizedPointsFixed(ScaleMode scale_mode, OffsetMode offset_mode);
    void add_point(const FixedArray<float, 2>& p);
    FixedArray<float, 2, 3> normalization_matrix() const;
    NormalizedPointsFixed chained(ScaleMode scale_mode, OffsetMode offset_mode) const;
private:
    FixedArray<float, 2> min_{float(INFINITY), float(INFINITY)};
    FixedArray<float, 2> max_{-float(INFINITY), -float(INFINITY)};
    ScaleMode scale_mode_;
    OffsetMode offset_mode_;
};

}
