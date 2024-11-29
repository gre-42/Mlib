#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized.hpp>
#include <Mlib/Math/Abs.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Max.hpp>
#include <Mlib/Math/Sqrt.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Uninitialized.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

template <class TDir, class TPos, size_t tndim>
class PlaneNd;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

template <class TPos, size_t tndim>
class BoundingSphere {
public:
    BoundingSphere(Uninitialized)
        : center_{ uninitialized }
    {}
    BoundingSphere(
        const FixedArray<TPos, tndim>& center,
        const TPos& radius)
        : center_{ center }
        , radius_{ radius }
    {}
    explicit BoundingSphere(const FixedArray<TPos, 2, tndim>& line)
        : BoundingSphere{ from_iterator(line.row_begin(), line.row_end()) }
    {}
    template <size_t tnpoints>
    explicit BoundingSphere(const FixedArray<TPos, tnpoints, tndim>& points)
        : BoundingSphere{ from_iterator(points.row_begin(), points.row_end()) }
    {}
    template <class TIterable>
    static BoundingSphere from_center_and_iterator(
        const FixedArray<TPos, tndim>& center,
        const TIterable& iterator_begin,
        const TIterable& iterator_end)
    {
        auto radius2 = funpack_t<TPos>(0);
        for (auto it = iterator_begin; it != iterator_end; ++it) {
            radius2 = max(radius2, sum(squared(*it - center)));
        }
        return BoundingSphere(center, (TPos)std::sqrt(radius2));
    }
    template <class TIterable>
    static BoundingSphere from_iterator(
        const TIterable& iterator_begin,
        const TIterable& iterator_end)
    {
        size_t nelements = 0;
        FixedArray<TPos, tndim> center((TPos)0);
        for (auto it = iterator_begin; it != iterator_end; ++it) {
            center += *it;
            ++nelements;
        }
        if (nelements == 0) {
            THROW_OR_ABORT("Bounding sphere received no elements");
        }
        center /= nelements;
        return from_center_and_iterator(center, iterator_begin, iterator_end);
    }
    bool contains(const FixedArray<TPos, tndim>& other, const TPos& tolerance = TPos(0)) const {
        return sum(squared(other - center_)) <= squared(radius_ + tolerance);
    }
    bool intersects(const BoundingSphere& other) const {
        return sum(squared(other.center_ - center_)) <= squared(other.radius_ + radius_);
    }
    template <class TDir>
    bool intersects(const PlaneNd<TDir, TPos, tndim>& plane) const {
        using fa = funpack_t<TPos>;
        TPos dist = dot0d(plane.normal.template casted<fa>(), center_.template casted<fa>()) + (fa)plane.intercept;
        return abs(dist) <= radius_;
    }
    void extend(const BoundingSphere& other) {
        radius_ = std::max(radius_, std::sqrt(sum(squared(other.center_ - center_))) + other.radius_);
    }
    template <class TTDir, class TTPos>
    BoundingSphere<TPos, tndim> transformed(const TransformationMatrix<TTDir, TTPos, tndim>& transformation_matrix) const {
        return BoundingSphere<TPos, tndim>{
            transformation_matrix.transform(center_.template casted<TTPos>()).template casted<TPos>(),
            radius_};
    }
    template <class TTDir, class TTPos>
    BoundingSphere<TPos, tndim> itransformed(const TransformationMatrix<TTDir, TTPos, tndim>& transformation_matrix) const {
        return BoundingSphere<TPos, tndim>{
            transformation_matrix.itransform(center_.template casted<TTPos>()).template casted<TPos>(),
            radius_};
    }
    inline const FixedArray<TPos, tndim>& center() const {
        return center_;
    }
    inline const TPos& radius() const {
        return radius_;
    }
    template <class TResultData>
    BoundingSphere<TResultData, tndim> casted() const {
        return BoundingSphere<TResultData, tndim>(
            center_.template casted<TResultData>(),
            (TResultData)radius_);
    }
private:
    FixedArray<TPos, tndim> center_;
    TPos radius_;
};

template <class TPos, size_t tndim>
using UBoundingSphere = DefaultUnitialized<BoundingSphere<TPos, tndim>>;

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
