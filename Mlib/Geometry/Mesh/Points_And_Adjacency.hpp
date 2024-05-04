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

template <class TPoint>
struct PointsAndAdjacency {
    using TData = typename TPoint::value_type;
    static const size_t tndim = TPoint::ndim();
    std::vector<TPoint> points;
    SparseArrayCcs<TData> adjacency;

    PointsAndAdjacency() = default;
    explicit PointsAndAdjacency(size_t npoints);
    ~PointsAndAdjacency() = default;

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
    PointsAndAdjacency concatenated(const PointsAndAdjacency& other) const;
    void insert(const PointsAndAdjacency& other);
    void merge_neighbors(TData radius);
    template <class TSize>
    void plot(Svg<TSize>& svg, float line_width = 1.5) const;
    void plot(const std::string& filename, float width, float height, float line_width = 1.5) const;
};

}
