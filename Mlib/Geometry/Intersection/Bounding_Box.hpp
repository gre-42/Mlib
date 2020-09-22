#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <iosfwd>
#include <string>

namespace Mlib {

template <class TData, size_t tndim>
class BoundingSphere;

template <class TData, size_t tndim>
class BoundingBox {
public:
    BoundingBox()
    : min_(INFINITY),
      max_(-INFINITY)
    {}
    BoundingBox(const FixedArray<TData, tndim>& min, const FixedArray<TData, tndim>& max)
    : min_{min},
      max_{max}
    {}
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
    bool intersects(const BoundingSphere<TData, tndim>& sphere) const {
        return all(sphere.center() >= min_ - sphere.radius()) &&
               all(sphere.center() <= max_ + sphere.radius());
    }
    void extend(const BoundingBox& other) {
        min_ = minimum(min_, other.min_);
        max_ = maximum(max_, other.max_);
    }
    FixedArray<TData, tndim> size() {
        return max_ - min_;
    }
    void print(std::ostream& ostr, size_t rec = 0) const {
        std::string indent(rec, ' ');
        ostr << indent << "bounds " << min_ << " -- " << max_ << std::endl;
    }
    inline const FixedArray<TData, tndim>& min() const {
        return min_;
    }
    inline const FixedArray<TData, tndim>& max() const {
        return max_;
    }
private:
    FixedArray<TData, tndim> min_;
    FixedArray<TData, tndim> max_;
};

template <class TData, size_t tndim>
std::ostream& operator << (std::ostream& ostr, const BoundingBox<TData, tndim>& bounding_box) {
    bounding_box.print(ostr);
    return ostr;
}

}
