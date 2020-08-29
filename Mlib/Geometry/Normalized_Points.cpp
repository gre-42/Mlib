#include "Normalized_Points.hpp"
#include <Mlib/Stats/Quantile.hpp>

using namespace Mlib;

NormalizedPoints::NormalizedPoints(bool preserve_aspect_ratio, bool centered)
: preserve_aspect_ratio_(preserve_aspect_ratio),
  centered_(centered) {}

void NormalizedPoints::add_point(const Array<float>& y)
{
    assert(y.length() == 3);
    min_x_ = std::min(y(0), min_x_);
    max_x_ = std::max(y(0), max_x_);
    min_y_ = std::min(y(1), min_y_);
    max_y_ = std::max(y(1), max_y_);
    assert(std::abs(y(2) - 1) < 1e-12);
}

void NormalizedPoints::add_points_quantile(const Array<float>& p, float q) {
    assert(p.ndim() == 2);
    assert(p.shape(1) == 3);
    assert(q < 0.5);
    Array<float> pT = p.T();
    Array<float> qx = quantiles(pT[0], Array<float>{q, 1 - q});
    Array<float> qy = quantiles(pT[1], Array<float>{q, 1 - q});
    for(const auto& pp : p) {
        if (pp(0) >= qx(0) && pp(1) >= qy(0) &&
            pp(0) <= qx(1) && pp(1) <= qy(1))
        {
            add_point(pp);
        }
    }
}

Array<float> NormalizedPoints::normalization_matrix() const {
    // dx: difference
    // sx: sum
    float dx = centered_ ? (max_x_ - min_x_) / 2 : (max_x_ - min_x_);
    float sx = centered_ ? (min_x_ + max_x_) / 2 : min_x_;
    float dy = centered_ ? (max_y_ - min_y_) / 2 : (max_y_ - min_y_);
    float sy = centered_ ? (min_y_ + max_y_) / 2 : min_y_;
    if (preserve_aspect_ratio_) {
        dx = dy = std::max(dx, dy);
    }
    return Array<float>{
        {1 / dx, 0, -sx / dx}, // (p - sx) / dx = p/dx - sx/dx
        {0, 1 / dy, -sy / dy}, // (p - sy) / dy = p/dy - sy/dy
        {0, 0, 1}};
}
