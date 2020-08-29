#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

class NormalizedPoints {
public:
    explicit NormalizedPoints(bool preserve_aspect_ratio, bool centered);
    void add_point(const Array<float>& p);
    void add_points_quantile(const Array<float>& p, float q);
    Array<float> normalization_matrix() const;
private:
    float min_x_ = std::numeric_limits<float>::infinity();
    float max_x_ = -std::numeric_limits<float>::infinity();
    float min_y_ = std::numeric_limits<float>::infinity();
    float max_y_ = -std::numeric_limits<float>::infinity();
    bool preserve_aspect_ratio_;
    bool centered_;
};

}
