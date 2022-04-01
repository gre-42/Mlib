#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Images/Svg.hpp>

namespace Mlib {

enum class SubdivisionType {
    SYMMETRIC,
    ASYMMETRIC,
    MAKE_SYMMETRIC
};

template <class TData, size_t tndim>
struct PointsAndAdjacency {
    std::vector<FixedArray<TData, tndim>> points;
    SparseArrayCcs<TData> adjacency;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(points);
        archive(adjacency);
    }
    void update_adjacency();
    template <class TCalculateIntermediatePoints>
    void subdivide(
        const TCalculateIntermediatePoints& calculate_intermediate_points,
        SubdivisionType subdivision_type);
    template <class TSize>
    void plot(Svg<TSize>& svg, float line_width = 1.5) const;
    void plot(const std::string& filename, float width, float height, float line_width = 1.5) const;
};

}
