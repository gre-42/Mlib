#include "Fuzzy_Set_Of_Points.hpp"
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>

namespace Mlib {

template <class TData, size_t tndim>
FuzzySetOfPoints<TData, tndim>::FuzzySetOfPoints(
    const TData& merge_radius,
    const TData& error_radius)
    : bvh_{ fixed_full<TData, tndim>(merge_radius), 17 }
    , bvh_size_{ 0 }
    , merge_radius_{ merge_radius }
    , error_radius_{ error_radius }
{}

template <class TData, size_t tndim>
bool FuzzySetOfPoints<TData, tndim>::insert(const FixedArray<TData, tndim>& p, size_t& index) {
    const std::pair<FixedArray<TData, tndim>, size_t>* neighbor;
    auto min_dist2 = bvh_.min_distance(
        p,
        merge_radius_,
        [&p](const auto& a){return sum(squared(a.first - p));},
        &neighbor);
    if (min_dist2.has_value() && (*min_dist2 < squared(merge_radius_))) {
        index = neighbor->second;
        return false;
    } else if (min_dist2.has_value() && (*min_dist2 < squared(error_radius_))) {
        THROW_OR_ABORT2((PointException{ p, "Detected problematic fuzzy point distance" }));
    } else {
        index = bvh_size_++;
        bvh_.insert(
            AxisAlignedBoundingBox<TData, tndim>::from_point(p),
            std::pair<FixedArray<TData, tndim>, size_t>{ p, index });
        return true;
    }
}

template <class TData, size_t tndim>
void FuzzySetOfPoints<TData, tndim>::optimize_search_time(std::ostream& ostr) const {
    bvh_.optimize_search_time(BvhDataRadiusType::ZERO, ostr);
}

}
