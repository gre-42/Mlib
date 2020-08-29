#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Min_Max.hpp>

namespace Mlib {

template <class TData, size_t tndim>
class BoundingBox {
public:
    BoundingBox() = default;
    BoundingBox(const FixedArray<TData, tndim>& point)
    : min_{point},
      max_{point}
    {}
    BoundingBox(const FixedArray<FixedArray<TData, tndim>, tndim>& triangle)
    : BoundingBox()
    {
        for(size_t i = 0; i < tndim; ++i) {
            extend(triangle(i));
        }
    }
    bool intersects(const BoundingBox& other) const {
        return all(max_ >= other.min_) && all(min_ <= other.max_);
    }
    void extend(const BoundingBox& other) {
        min_ = minimum(min_, other.min_);
        max_ = maximum(max_, other.max_);
    }
    FixedArray<TData, tndim> min_ = fixed_full<TData, tndim>(INFINITY);
    FixedArray<TData, tndim> max_ = fixed_full<TData, tndim>(-INFINITY);
};

}
