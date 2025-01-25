#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>

namespace Mlib {

template <class TColor>
void draw_circle_points(Array<TColor>& image, float angle0, size_t na, int nz) {
    assert(image.ndim() == 2);
    for (size_t ia = 0; ia < na; ia++) {
        float dangle0 = float(M_PI) * float(ia) / float(na);
        for (int iz = -nz; iz <= nz; iz++) {
            float angle1 = float(M_PI) * float(iz) / float(nz);
            Array<float> point3d;
            point3d.resize(3);
            point3d(0) = std::cos(angle0 + dangle0) * std::cos(angle1);
            point3d(1) = std::sin(angle1);
            point3d(2) = std::sin(angle0 + dangle0) * std::cos(angle1);
            Array<float> point2d;
            point2d = point3d;
            size_t r = static_cast<size_t>(std::round((0.4f * point2d(id1) + 0.5f) * image.shape(0)));
            size_t c = static_cast<size_t>(std::round((0.4f * point2d(id0) + 0.5f) * image.shape(1)));
            // lerr() << r << " " << c;
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
    const FixedArray<size_t, 2>& field_shape,
    const FixedArray<size_t, 2>& center,
    const Array<float>& velocity,
    size_t length,
    const std::function<void(const FixedArray<size_t, 2>& index)>& operation,
    bool freeze_velocity = false)
{
    assert(velocity.ndim() == 3);
    assert(velocity.shape(0) == 2);
    assert(center.ndim() == 2);
    assert(all(velocity.shape().erased_first() == ArrayShape{ field_shape(0), field_shape(1) }));
    FixedArray<float, 2> fpos{i2a(center)};
    if (any(center >= field_shape)) {
        return;
    }
    for (size_t i = 0; i < length; ++i) {
        FixedArray<size_t, 2> ipos{a2i(fpos)};
        if (any(ipos >= field_shape)) {
            break;
        }
        //(*this)(ipos) = color;
        operation(ipos);
        FixedArray<size_t, 2> epos = freeze_velocity ? center : ipos;
        auto v = FixedArray<float, 2>{
            velocity(0, epos(0), epos(1)),
            velocity(1, epos(0), epos(1))};
        if (any(Mlib::isnan(v))) {
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
    const FixedArray<size_t, 2>& center,
    size_t size,
    const TColor& value)
{
    assert(image.ndim() == 2);
    for (size_t r = center(0) - size; r != center(0) + size + 1; ++r) {
        if (r >= image.shape(0)) {
            continue;
        }
        for (size_t c = center(1) - size; c != center(1) + size + 1; ++c) {
            if (c >= image.shape(1)) {
                continue;
            }
            image(r, c) = value;
        }
    }
}

template<class TColor>
void draw_empty_rect(
    Array<TColor>& image,
    const FixedArray<size_t, 2>& center,
    size_t size,
    const TColor& value)
{
    assert(image.ndim() == 2);
    for (size_t r = center(0) - size; r != center(0) + size + 1; ++r) {
        if (r >= image.shape(0)) {
            continue;
        }
        for (size_t c = center(1) - size; c != center(1) + size + 1; ++c) {
            if (c >= image.shape(1)) {
                continue;
            }
            if ((r == center(0) - size) || (r == center(0) + size)
             || (c == center(1) - size) || (c == center(1) + size))
            {
                image(r, c) = value;
            }
        }
    }
}

template<class TColor>
void draw_line_ext(
    Array<TColor>& image,
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    const TColor& color,
    bool infinite,
    const TColor* short_line_color = nullptr)
{
    auto draw_point = [&image, &thickness](const FixedArray<float, 2>& p, const TColor& color){
        FixedArray<size_t, 2> index{ fi2i(p(0)), fi2i(p(1)) };
        if (any(index >= FixedArray<size_t, 2>{ image.shape(0), image.shape(1) })) {
            return false;
        }
        draw_fill_rect(image, index, thickness, color);
        return true;
    };
    FixedArray<float, 2> v = to - from;
    float len = max(abs(v));
    if (std::abs(len) < 1e-12) {
        if (short_line_color == nullptr) {
            THROW_OR_ABORT("draw_infinite_line: from ~= to");
        }
        draw_point(from, *short_line_color);
        return;
    }
    v /= len;
    FixedArray<float, 2> p = from;
    for (size_t i = 0; infinite || i < (size_t)len; ++i) {
        if (!draw_point(p, color)) {
            break;
        }
        p += v;
    }
}

template<class TColor>
void draw_points_as_boxes(
    const Array<FixedArray<float, 2>>& feature_points,
    Array<TColor>& image,
    size_t size,
    const TColor& value)
{
    assert(image.ndim() == 2);
    assert(feature_points.ndim() == 1);
    for (const FixedArray<float, 2>& feature_point : feature_points.flat_iterable()) {
        //assert(feature_point(0) >= 0);
        //assert(feature_point(0) <= image.shape(1) - 1);
        //assert(feature_point(1) >= 0);
        //assert(feature_point(1) <= image.shape(0) - 1);
        FixedArray<size_t, 2> index{ a2i(feature_point) };
        draw_fill_rect(image, index, size, value);
    }
}

}
