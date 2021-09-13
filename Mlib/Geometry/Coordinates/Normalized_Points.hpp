#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

class NormalizedPoints {
public:
    explicit NormalizedPoints(bool preserve_aspect_ratio, bool centered);
    void add_point(const FixedArray<float, 2>& p);
    void add_points_quantile(const Array<FixedArray<float, 2>>& p, float q);
    FixedArray<float, 3, 3> normalization_matrix() const;
private:
    float min_x_ = std::numeric_limits<float>::infinity();
    float max_x_ = -std::numeric_limits<float>::infinity();
    float min_y_ = std::numeric_limits<float>::infinity();
    float max_y_ = -std::numeric_limits<float>::infinity();
    bool preserve_aspect_ratio_;
    bool centered_;
};

}
