#pragma once
#include <Mlib/Geometry/Primitives/Axis_Aligned_Bounding_Box.hpp>

namespace Mlib {

enum class ScaleMode {
    NONE,
    PRESERVE_ASPECT_RATIO,
    DIAGONAL
};

enum class OffsetMode {
    CENTERED,
    MINIMUM
};

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

template <class TData>
class NormalizedPointsFixed {
public:
    NormalizedPointsFixed(ScaleMode scale_mode, OffsetMode offset_mode);
    void add_point(const FixedArray<TData, 2>& p);
    void set_min(const FixedArray<TData, 2>& p);
    void set_max(const FixedArray<TData, 2>& p);
    TransformationMatrix<TData, TData, 2> normalization_matrix() const;
    NormalizedPointsFixed chained(ScaleMode scale_mode, OffsetMode offset_mode) const;
    const FixedArray<TData, 2>& min() const;
    const FixedArray<TData, 2>& max() const;
    const AxisAlignedBoundingBox<TData, 2>& aabb() const;
private:
    AxisAlignedBoundingBox<TData, 2> aabb_;
    ScaleMode scale_mode_;
    OffsetMode offset_mode_;
};

}
