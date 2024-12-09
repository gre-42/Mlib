#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>

namespace Mlib {

template <class TData, size_t tndim>
class FuzzySetOfPoints {
public:
    explicit FuzzySetOfPoints(
        const TData& merge_radius,
        const TData& error_radius);
    bool insert(const FixedArray<TData, tndim>& p, size_t& index);
    void optimize_search_time(std::ostream& ostr) const;
private:
    Bvh<TData, tndim, std::pair<FixedArray<TData, tndim>, size_t>> bvh_;
    size_t bvh_size_;
    TData merge_radius_;
    TData error_radius_;
};

}
