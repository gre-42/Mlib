#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

namespace Mlib {

template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

struct GlLookatAabb {
    FixedArray<float, 3, 3> extrinsic_R;
    AxisAlignedBoundingBox<float, 2> sensor_aabb;
    float near_plane;
    float far_plane;
};

std::optional<GlLookatAabb> gl_lookat_aabb(
    const FixedArray<double, 3>& camera_position,
    const TransformationMatrix<float, double, 3>& object_model_matrix,
    const AxisAlignedBoundingBox<double, 3>& object_aabb);

}
