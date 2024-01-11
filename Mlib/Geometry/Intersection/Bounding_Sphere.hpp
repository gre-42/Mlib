#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

template <class TData, size_t tndim>
class PlaneNd;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

template <class TData, size_t tndim>
class BoundingSphere {
public:
    BoundingSphere(
        const FixedArray<TData, tndim>& center,
        const TData& radius)
    : center_{center},
      radius_{radius}
    {}
    explicit BoundingSphere(const FixedArray<FixedArray<TData, tndim>, 2>& line)
    : BoundingSphere{from_iterator(line.flat_begin(), line.flat_end())}
    {}
    template <size_t tnpoints>
    explicit BoundingSphere(const FixedArray<FixedArray<TData, tndim>, tnpoints>& points)
    : BoundingSphere{from_iterator(points.flat_begin(), points.flat_end())}
    {}
    template <class TIterable>
    static BoundingSphere from_center_and_iterator(
        const FixedArray<TData, tndim>& center,
        const TIterable& iterator_begin,
        const TIterable& iterator_end)
    {
        TData radius2 = 0;
        for (auto it = iterator_begin; it != iterator_end; ++it) {
            radius2 = std::max(radius2, sum(squared(*it - center)));
        }
        return BoundingSphere(center, std::sqrt(radius2));
    }
    template <class TIterable>
    static BoundingSphere from_iterator(
        const TIterable& iterator_begin,
        const TIterable& iterator_end)
    {
        size_t nelements = 0;
        FixedArray<TData, tndim> center(0);
        for (auto it = iterator_begin; it != iterator_end; ++it) {
            center += *it;
            ++nelements;
        }
        if (nelements == 0) {
            THROW_OR_ABORT("Bounding sphere received no elements");
        }
        center /= (TData)nelements;
        return from_center_and_iterator(center, iterator_begin, iterator_end);
    }
    bool contains(const FixedArray<TData, tndim>& other, const TData& tolerance = TData(0)) const {
        return sum(squared(other - center_)) <= squared(radius_ + tolerance);
    }
    bool intersects(const BoundingSphere& other) const {
        return sum(squared(other.center_ - center_)) <= squared(other.radius_ + radius_);
    }
    bool intersects(const PlaneNd<TData, tndim>& plane) const {
        TData dist = dot0d(plane.normal, center_) + plane.intercept;
        return std::abs(dist) <= radius_;
    }
    template <class TDir, class TPos>
    BoundingSphere<TPos, tndim> transformed(const TransformationMatrix<TDir, TPos, tndim>& transformation_matrix) const {
        return BoundingSphere<TPos, tndim>{
            transformation_matrix.transform(center_ TEMPLATEV casted<TPos>()),
            radius_ * transformation_matrix.get_scale()};
    }
    inline const FixedArray<TData, tndim>& center() const {
        return center_;
    }
    inline const TData& radius() const {
        return radius_;
    }
private:
    FixedArray<TData, tndim> center_;
    TData radius_;
};

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
