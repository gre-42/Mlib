#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized.hpp>
#include <Mlib/Math/Abs.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Max.hpp>
#include <Mlib/Math/Sqrt.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
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
        : center{ uninitialized }
    {}
    BoundingSphere(
        const FixedArray<TPos, tndim>& center,
        const TPos& radius)
        : center{ center }
        , radius{ radius }
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
        using I = funpack_t<TPos>;
        FixedArray<I, tndim> center((I)0);
        for (auto it = iterator_begin; it != iterator_end; ++it) {
            center += it->template casted<I>();
            ++nelements;
        }
        if (nelements == 0) {
            THROW_OR_ABORT("Bounding sphere received no elements");
        }
        center /= integral_to_float<I>(nelements);
        return from_center_and_iterator(center.template casted<TPos>(), iterator_begin, iterator_end);
    }
    bool contains(const FixedArray<TPos, tndim>& other, const TPos& tolerance = TPos(0.f)) const {
        return sum(squared(other - center)) <= squared(radius + tolerance);
    }
    bool intersects(const BoundingSphere& other, const TPos& tolerance = TPos(0.f)) const {
        return sum(squared(other.center - center)) <= squared(other.radius + radius + tolerance);
    }
    template <class TDir>
    bool intersects(const PlaneNd<TDir, TPos, tndim>& plane) const {
        using fa = funpack_t<TPos>;
        auto dist = (TPos)dot0d(plane.normal.template casted<fa>(), center.template casted<fa>()) + plane.intercept;
        return abs(dist) <= radius;
    }
    void extend(const BoundingSphere& other) {
        radius = std::max(radius, (TPos)std::sqrt(sum(squared(other.center - center))) + other.radius);
    }
    template <class TTDir, class TTPos>
    BoundingSphere<TPos, tndim> transformed(const TransformationMatrix<TTDir, TTPos, tndim>& transformation_matrix) const {
        return BoundingSphere<TPos, tndim>{
            transformation_matrix.transform(center.template casted<TTPos>()).template casted<TPos>(),
            radius};
    }
    template <class TTDir, class TTPos>
    BoundingSphere<TPos, tndim> itransformed(const TransformationMatrix<TTDir, TTPos, tndim>& transformation_matrix) const {
        return BoundingSphere<TPos, tndim>{
            transformation_matrix.itransform(center.template casted<TTPos>()).template casted<TPos>(),
            radius};
    }
    template <class TResultData>
    BoundingSphere<TResultData, tndim> casted() const {
        return BoundingSphere<TResultData, tndim>(
            center.template casted<TResultData>(),
            (TResultData)radius);
    }
    bool operator == (const BoundingSphere& other) const {
        return all(center == other.center) &&
               (radius == other.radius);
    }
    FixedArray<TPos, tndim> center;
    TPos radius;
};

template <class TPos, size_t tndim>
BoundingSphere<TPos, tndim> operator + (
    const BoundingSphere<TPos, tndim>& b,
    const FixedArray<TPos, tndim>& x)
{
    return { b.center + x, b.radius };
}

template <class TPos, size_t tndim>
using UBoundingSphere = DefaultUnitialized<BoundingSphere<TPos, tndim>>;

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
