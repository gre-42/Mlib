#pragma once
#include <Mlib/Geometry/Intersection/Convex_Polygon.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Stats/Clamped.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Uninitialized.hpp>
#include <array>
#include <iosfwd>
#include <string>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

template <class TData, size_t tndim>
class AxisAlignedBoundingBox {
    template <class TData2, size_t tndim2>
    friend class AxisAlignedBoundingBox; 
public:
    AxisAlignedBoundingBox(Uninitialized)
        : min_{ uninitialized }
        , max_{ uninitialized }
    {}
    static AxisAlignedBoundingBox empty() {
        return AxisAlignedBoundingBox{
            fixed_full<TData, tndim>(INFINITY),
            fixed_full<TData, tndim>(-INFINITY)};
    }
    static AxisAlignedBoundingBox from_min_max(
        const FixedArray<TData, tndim>& min,
        const FixedArray<TData, tndim>& max) {
        return AxisAlignedBoundingBox{ min, max };
    }
    static AxisAlignedBoundingBox from_point(const FixedArray<TData, tndim>& point) {
        return AxisAlignedBoundingBox{ point, point };
    }
    static AxisAlignedBoundingBox from_center_and_radius(
        const FixedArray<TData, tndim>& center,
        const TData& radius)
    {
        return AxisAlignedBoundingBox{
            center - radius,
            center + radius };
    }
    template <size_t tnpoints>
    static AxisAlignedBoundingBox from_points(
        const FixedArray<TData, tnpoints, tndim>& points)
    {
        return from_iterator(points.row_begin(), points.row_end());
    }
    static AxisAlignedBoundingBox from_points(
        const FixedArray<TData, tndim>& a,
        const FixedArray<TData, tndim>& b)
    {
        return AxisAlignedBoundingBox{ minimum(a, b), maximum(a, b) };
    }
    template <class TIterator>
    static AxisAlignedBoundingBox from_iterator(
        const TIterator& iterator_begin,
        const TIterator& iterator_end)
    {
        auto result = empty();
        for (auto it = iterator_begin; it != iterator_end; ++it) {
            result.extend(*it);
        }
        return result;
    }
    bool intersects(const AxisAlignedBoundingBox& other) const {
        // return all(max_ >= other.min_) && all(min_ <= other.max_);
        for (size_t i = 0; i < tndim; ++i) {
            if (max_(i) < other.min_(i)) {
                return false;
            }
            if (min_(i) > other.max_(i)) {
                return false;
            }
        }
        return true;
    }
    bool contains(const AxisAlignedBoundingBox& other) const {
        // return all(max_ >= other.max_) && all(min_ <= other.min_);
        for (size_t i = 0; i < tndim; ++i) {
            if (max_(i) < other.max_(i)) {
                return false;
            }
            if (min_(i) > other.min_(i)) {
                return false;
            }
        }
        return true;
    }
    bool contains(const FixedArray<TData, tndim>& point) const {
        // return all(max_ >= point) && all(min_ <= point);
        for (size_t i = 0; i < tndim; ++i) {
            if (max_(i) < point(i)) {
                return false;
            }
            if (min_(i) > point(i)) {
                return false;
            }
        }
        return true;
    }
    FixedArray<TData, tndim> closest_point(const FixedArray<TData, tndim>& point) const {
        return clamped(point, min(), max());
    }
    void extend(const AxisAlignedBoundingBox& other) {
        min_ = minimum(min_, other.min_);
        max_ = maximum(max_, other.max_);
    }
    void extend(const FixedArray<TData, tndim>& point) {
        min_ = minimum(min_, point);
        max_ = maximum(max_, point);
    }
    template <class TDir, class TPos>
    AxisAlignedBoundingBox<TPos, tndim> transformed(
        const TransformationMatrix<TDir, TPos, tndim>& transformation_matrix) const
    {
        auto result = empty();
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
    FixedArray<TData, tndim> center() const {
        return (min_ + max_) * TData{ 0.5 };
    }
    FixedArray<TData, tndim> size() const {
        return max_ - min_;
    }
    void print(std::ostream& ostr, size_t rec = 0) const {
        std::string indent(rec, ' ');
        ostr << indent << "bounds " << min_ << " -- " << max_;
    }
    inline const FixedArray<TData, tndim>& min() const {
        return min_;
    }
    inline const FixedArray<TData, tndim>& max() const {
        return max_;
    }
    inline const TData& min(size_t i) const {
        return min_(i);
    }
    inline const TData& max(size_t i) const {
        return max_(i);
    }
    template <class TOperation>
    bool for_each_corner(const TOperation& op) const {
        FixedArray<TData, tndim> corner = uninitialized;
        return for_each_corner(op, 0, corner);
    }
    template <class TOperation>
    bool for_each_edge(const TOperation& op) const {
        static_assert(tndim == 3);
        const auto edges = FixedArray<TData, 12, 2, 3>{
            // 0, 0, 0
            FixedArray<TData, 2, 3>{
                FixedArray<TData, 3>{min(0), min(1), min(2)},
                FixedArray<TData, 3>{min(0), min(1), max(2)}
            },
            FixedArray<TData, 2, 3>{
                FixedArray<TData, 3>{min(0), min(1), min(2)},
                FixedArray<TData, 3>{min(0), max(1), min(2)}
            },
            FixedArray<TData, 2, 3>{
                FixedArray<TData, 3>{min(0), min(1), min(2)},
                FixedArray<TData, 3>{max(0), min(1), min(2)}
            },
            // 0, 0, 1
            FixedArray<TData, 2, 3>{
                FixedArray<TData, 3>{min(0), min(1), max(2)},
                FixedArray<TData, 3>{min(0), max(1), max(2)}
            },
            FixedArray<TData, 2, 3>{
                FixedArray<TData, 3>{min(0), min(1), max(2)},
                FixedArray<TData, 3>{max(0), min(1), max(2)}
            },
            // 0, 1, 0
            FixedArray<TData, 2, 3>{
                FixedArray<TData, 3>{min(0), max(1), min(2)},
                FixedArray<TData, 3>{min(0), max(1), max(2)}
            },
            FixedArray<TData, 2, 3>{
                FixedArray<TData, 3>{min(0), max(1), min(2)},
                FixedArray<TData, 3>{max(0), max(1), min(2)}
            },
            // 0, 1, 1
            FixedArray<TData, 2, 3>{
                FixedArray<TData, 3>{min(0), max(1), max(2)},
                FixedArray<TData, 3>{max(0), max(1), max(2)}
            },
            // 1, 0, 0
            FixedArray<TData, 2, 3>{
                FixedArray<TData, 3>{max(0), min(1), min(2)},
                FixedArray<TData, 3>{max(0), min(1), max(2)}
            },
            FixedArray<TData, 2, 3>{
                FixedArray<TData, 3>{max(0), min(1), min(2)},
                FixedArray<TData, 3>{max(0), max(1), min(2)}
            },
            // 1, 0, 1
            FixedArray<TData, 2, 3>{
                FixedArray<TData, 3>{max(0), min(1), max(2)},
                FixedArray<TData, 3>{max(0), max(1), max(2)}
            },
            // 1, 1, 0
            FixedArray<TData, 2, 3>{
                FixedArray<TData, 3>{max(0), min(1), max(2)},
                FixedArray<TData, 3>{max(0), max(1), max(2)}
            }
        };
        for (const auto& e : edges.row_iterable()) {
            if (!op(e)) {
                return false;
            }
        }
        return true;
    }
    template <class TOperation>
    bool for_each_face(const TOperation& op) const {
        static_assert(tndim == 3);
        auto faces = std::array<ConvexPolygon3D<TData, 4>, 6>{
            // x-
            ConvexPolygon3D<TData, 4>{FixedArray<TData, 4, 3>{
                FixedArray<TData, 3>{min(0), min(1), min(2)},
                FixedArray<TData, 3>{min(0), max(1), min(2)},
                FixedArray<TData, 3>{min(0), max(1), max(2)},
                FixedArray<TData, 3>{min(0), min(1), max(2)}}},
            // x+
            ConvexPolygon3D<TData, 4>{FixedArray<TData, 4, 3>{
                FixedArray<TData, 3>{max(0), min(1), min(2)},
                FixedArray<TData, 3>{max(0), max(1), min(2)},
                FixedArray<TData, 3>{max(0), max(1), max(2)},
                FixedArray<TData, 3>{max(0), min(1), max(2)}}},
            // y-
            ConvexPolygon3D<TData, 4>{FixedArray<TData, 4, 3>{
                FixedArray<TData, 3>{min(0), min(1), min(2)},
                FixedArray<TData, 3>{max(0), min(1), min(2)},
                FixedArray<TData, 3>{max(0), min(1), max(2)},
                FixedArray<TData, 3>{min(0), min(1), max(2)}}},
            // y+
            ConvexPolygon3D<TData, 4>{FixedArray<TData, 4, 3>{
                FixedArray<TData, 3>{min(0), max(1), min(2)},
                FixedArray<TData, 3>{max(0), max(1), min(2)},
                FixedArray<TData, 3>{max(0), max(1), max(2)},
                FixedArray<TData, 3>{min(0), max(1), max(2)}}},
            // z-
            ConvexPolygon3D<TData, 4>{FixedArray<TData, 4, 3>{
                FixedArray<TData, 3>{min(0), min(1), min(2)},
                FixedArray<TData, 3>{max(0), min(1), min(2)},
                FixedArray<TData, 3>{max(0), max(1), min(2)},
                FixedArray<TData, 3>{min(0), max(1), min(2)}}},
            // z+
            ConvexPolygon3D<TData, 4>{FixedArray<TData, 4, 3>{
                FixedArray<TData, 3>{min(0), min(1), max(2)},
                FixedArray<TData, 3>{max(0), min(1), max(2)},
                FixedArray<TData, 3>{max(0), max(1), max(2)},
                FixedArray<TData, 3>{min(0), max(1), max(2)}}}
        };
        for (const auto& f : faces) {
            if (!op(f)) {
                return false;
            }
        }
        return true;
    }
    template <class TResultData>
    AxisAlignedBoundingBox<TResultData, tndim> casted() const {
        return AxisAlignedBoundingBox<TResultData, tndim>(
            min_.template casted<TResultData>(),
            max_.template casted<TResultData>());
    }
private:
    AxisAlignedBoundingBox(const FixedArray<TData, tndim>& min, const FixedArray<TData, tndim>& max)
        : min_{min}
        , max_{max}
    {}
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
