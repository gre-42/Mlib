#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <cmath>

namespace Mlib {

template <class TColor>
void draw_circle_points(Array<TColor>& image, const float angle0, const size_t na, const int nz) {
    assert(image.ndim() == 2);
    for(size_t ia = 0; ia < na; ia++) {
        const float dangle0 = M_PI * static_cast<float>(ia) / na;
        for(int iz = -nz; iz <= nz; iz++) {
            const float angle1 = M_PI * static_cast<float>(iz) / nz;
            Array<float> point3d;
            point3d.resize(3);
            point3d(0) = std::cos(angle0 + dangle0) * std::cos(angle1);
            point3d(1) = std::sin(angle1);
            point3d(2) = std::sin(angle0 + dangle0) * std::cos(angle1);
            Array<float> point2d;
            point2d = point3d;
            const size_t r = static_cast<size_t>(std::round((0.4*point2d(id1) + 0.5) * image.shape(0)));
            const size_t c = static_cast<size_t>(std::round((0.4*point2d(id0) + 0.5) * image.shape(1)));
            // std::cerr << r << " " << c << std::endl;
            if (point3d(2) < 0 &&
                r < image.shape(0) &&
                c < image.shape(1))
            {
                image(r, c) = 0;
            }
        }
    }
}

inline void visit_streamline(
    const ArrayShape& field_shape,
    const ArrayShape& center,
    const Array<float>& velocity,
    size_t length,
    const std::function<void(const ArrayShape& index)>& operation,
    bool freeze_velocity = false)
{
    assert(velocity.ndim() == 3);
    assert(velocity.shape(0) == 2);
    assert(center.ndim() == 2);
    assert(all(velocity.shape().erased_first() == field_shape));
    Array<float> fpos{i2a(center)};
    if (any(center >= field_shape)) {
        return;
    }
    for(size_t i = 0; i < length; ++i) {
        ArrayShape ipos{a2i(fpos)};
        if (any(ipos >= field_shape)) {
            break;
        }
        //(*this)(ipos) = color;
        operation(ipos);
        auto v = Array<float>{
            velocity[0](freeze_velocity ? center : ipos),
            velocity[1](freeze_velocity ? center : ipos)};
        if (std::isnan(v(0)) || std::isnan(v(1))) {
            break;
        }
        auto ls = sum(squared(v));
        if (ls < 1e-12) {
            break;
        }
        fpos += (v / float(std::sqrt(ls)));
    }
}

template<class TColor>
void draw_fill_rect(
    Array<TColor>& image,
    const ArrayShape& center,
    size_t size,
    const TColor& value)
{
    assert(image.ndim() == 2);
    assert(center.ndim() == 2);
    for(size_t r = center(0) - size; r != center(0) + size + 1; ++r) {
        if (r >= image.shape(0)) {
            continue;
        }
        for(size_t c = center(1) - size; c != center(1) + size + 1; ++c) {
            if (c >= image.shape(1)) {
                continue;
            }
            image(r, c) = value;
        }
    }
}

template<class TColor>
void draw_line_ext(
    Array<TColor>& image,
    const Array<float>& from,
    const Array<float>& to,
    size_t thickness,
    const TColor& color,
    bool infinite)
{
    assert(all(from.shape() == ArrayShape{2}));
    assert(all(to.shape() == ArrayShape{2}));
    Array<float> v = to - from;
    float len = max(abs(v));
    if (std::abs(len) < 1e-12) {
        throw std::runtime_error("draw_infinite_line: from ~= to");
    }
    v /= len;
    Array<float> p;
    p = from;
    for(size_t i = 0; infinite || i < len; ++i) {
        ArrayShape index{fi2i(p(0)), fi2i(p(1))};
        if (any(index >= image.shape())) {
            break;
        }
        draw_fill_rect(image, index, thickness, color);
        p += v;
    }
}

template<class TColor>
void draw_points_as_boxes(
    const Array<float>& feature_points,
    Array<TColor>& image,
    size_t size,
    const TColor& value)
{
    assert(image.ndim() == 2);
    assert(feature_points.ndim() == 2);
    assert(feature_points.shape(1) == 2);
    for(const Array<float>& feature_point : feature_points) {
        assert(feature_point.ndim() == 1);
        assert(feature_point.length() == 2);
        //assert(feature_point(0) >= 0);
        //assert(feature_point(0) <= image.shape(1) - 1);
        //assert(feature_point(1) >= 0);
        //assert(feature_point(1) <= image.shape(0) - 1);
        ArrayShape index{a2i(feature_point)};
        draw_fill_rect(image, index, size, value);
    }
}

}
