#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Array/Sparse_Array.hpp>

namespace Mlib {

enum class SubdivisionType {
    SYMMETRIC,
    ASYMMETRIC,
    MAKE_SYMMETRIC
};

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TSize>
class Svg;

template <class TData, size_t tndim>
struct PointsAndAdjacency {
    std::vector<FixedArray<TData, tndim>> points;
    SparseArrayCcs<TData> adjacency;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(points);
        archive(adjacency);
    }
    void transform(const TransformationMatrix<float, double, 3>& m);
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
