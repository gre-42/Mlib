#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>

namespace Mlib {

template <class TData, size_t tndim>
class CloseNeighborDetector {
public:
    CloseNeighborDetector(
        const FixedArray<TData, tndim>& max_size,
        size_t level)
    : bvh_{max_size, level}
    {}
    bool contains_neighbor(const FixedArray<TData, tndim>& p, const TData& distance) {
        bool res = bvh_.has_neighbor(p, distance, [p](const auto& n){
            TData dist2 = sum(squared(p - n));
            if (dist2 == 0.) {
                return (TData)INFINITY;
            }
            return std::sqrt(dist2);
        });
        bvh_.insert(AxisAlignedBoundingBox<TData, tndim>{p}, p);
        return res;
    }
private:
    Bvh<TData, FixedArray<TData, tndim>, tndim> bvh_;
};

}
