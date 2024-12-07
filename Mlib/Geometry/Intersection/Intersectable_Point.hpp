#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Uninitialized.hpp>
#include <iosfwd>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

template <class TData, size_t tndim>
class IntersectablePoint {
public:
    IntersectablePoint(Uninitialized)
        : coordinates_{ uninitialized }
    {}
    explicit IntersectablePoint(const FixedArray<TData, tndim>& point)
        : coordinates_{ point }
    {}
    bool intersects(const AxisAlignedBoundingBox<TData, tndim>& other) const {
        return other.contains(coordinates_);
    }
    void print(std::ostream& ostr, size_t rec = 0) const {
        std::string indent(rec, ' ');
        ostr << indent << "coords " << coordinates_;
    }
private:
    FixedArray<TData, tndim> coordinates_;
};

template <class TData, size_t tndim>
inline bool intersects(
    const IntersectablePoint<TData, tndim>& a,
    const AxisAlignedBoundingBox<TData, tndim>& b)
{
    return a.intersects(b);
}

template <class TData, size_t tndim>
std::ostream& operator << (std::ostream& ostr, const IntersectablePoint<TData, tndim>& aabb) {
    aabb.print(ostr);
    return ostr;
}

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
