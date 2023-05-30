#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <iosfwd>
#include <string>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

template <class TData, size_t tndim>
class BoundingSphere;

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

template <class TData, size_t tndim>
class AxisAlignedBoundingBox {
public:
    AxisAlignedBoundingBox()
    : min_(INFINITY),
      max_(-INFINITY)
    {}
    AxisAlignedBoundingBox(const FixedArray<TData, tndim>& min, const FixedArray<TData, tndim>& max)
    : min_{min},
      max_{max}
    {}
    AxisAlignedBoundingBox(const FixedArray<TData, tndim>& point)
    : min_{point},
      max_{point}
    {}
    AxisAlignedBoundingBox(const FixedArray<TData, tndim>& center, const TData& radius)
    : min_{center - radius},
      max_{center + radius}
    {}
    template <size_t tnpoints>
    AxisAlignedBoundingBox(const FixedArray<FixedArray<TData, tndim>, tnpoints>& points)
    : AxisAlignedBoundingBox{points.flat_begin(), points.flat_end()}
    {}
    template <class TIterable>
    explicit AxisAlignedBoundingBox(
        const TIterable& iterable_begin,
        const TIterable& iterable_end)
    : AxisAlignedBoundingBox()
    {
        for (auto it = iterable_begin; it != iterable_end; ++it) {
            extend(*it);
        }
    }
    bool intersects(const AxisAlignedBoundingBox& other) const {
        return all(max_ >= other.min_) && all(min_ <= other.max_);
    }
    void extend(const AxisAlignedBoundingBox& other) {
        min_ = minimum(min_, other.min_);
        max_ = maximum(max_, other.max_);
    }
    template <class TDir, class TPos>
    AxisAlignedBoundingBox<TPos, tndim> transformed(
        const TransformationMatrix<TDir, TPos, tndim>& transformation_matrix) const
    {
        AxisAlignedBoundingBox<TPos, tndim> result;
        for_each_corner([&](const FixedArray<TData, tndim>& corner){
            result.extend(transformation_matrix.transform(corner));
            return true;
        });
        return result;
    }
    AxisAlignedBoundingBox translated(const FixedArray<TData, tndim>& translation) const
    {
        return AxisAlignedBoundingBox(min_ + translation, max_ + translation);
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
    template <class TOperation>
    bool for_each_corner(const TOperation& op) const {
        FixedArray<TData, tndim> corner;
        return for_each_corner(op, 0, corner);
    }
    template <class TResultData>
    AxisAlignedBoundingBox<TResultData, tndim> casted() const {
        return AxisAlignedBoundingBox<TResultData, tndim>(
            min_ TEMPLATEV casted<TResultData>(),
            max_ TEMPLATEV casted<TResultData>());
    }
private:
    template <class TOperation>
    bool for_each_corner(
        const TOperation& op,
        size_t ndim0,
        FixedArray<TData, tndim>& corner) const
    {
        static_assert(tndim != 0);
        if (ndim0 == tndim) {
            return op(corner);
        } else {
            corner(ndim0) = min_(ndim0);
            if (!for_each_corner(op, ndim0 + 1, corner)) {
                return false;
            }
            corner(ndim0) = max_(ndim0);
            if (!for_each_corner(op, ndim0 + 1, corner)) {
                return false;
            }
            return true;
        }
    }
    FixedArray<TData, tndim> min_;
    FixedArray<TData, tndim> max_;
};

template <class TData, size_t tndim>
std::ostream& operator << (std::ostream& ostr, const AxisAlignedBoundingBox<TData, tndim>& aabb) {
    aabb.print(ostr);
    return ostr;
}

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
