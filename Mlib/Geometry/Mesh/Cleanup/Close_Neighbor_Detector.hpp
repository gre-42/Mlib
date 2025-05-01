#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Math/Funpack.hpp>

namespace Mlib {

enum class DuplicateRule {
    IS_NEIGHBOR,
    NO_NEIGHBOR
};

template <class TData, size_t tndim>
class CloseNeighborDetector {
public:
    CloseNeighborDetector(
        const FixedArray<TData, tndim>& max_size,
        size_t level)
        : bvh_{ max_size, level }
    {}
    bool contains_neighbor(
        const FixedArray<TData, tndim>& p,
        const TData& distance, DuplicateRule duplicate_rule)
    {
        bool res = bvh_.has_neighbor2(p, distance, [&](const auto& n) -> funpack_t<TData> {
            auto dist2 = sum(squared(p - n));
            if ((duplicate_rule == DuplicateRule::NO_NEIGHBOR) &&
                (dist2 == 0.))
            {
                return INFINITY;
            }
            return dist2;
        });
        bvh_.insert(PointWithoutPayload{ p });
        return res;
    }
private:
    PointWithoutPayloadVectorBvh<TData, tndim> bvh_;
};

}
