#pragma once
#include <Mlib/Geometry/Cameras/Frustum_Camera_Config.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib {

template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

struct GlLookatAabb {
    bool is_valid;
    FixedArray<float, 3, 3> extrinsic_R;
    FrustumCameraConfig frustum_camera_config;
    int width;
    int height;
};

std::optional<GlLookatAabb> gl_lookat_aabb(
    float dpi,
    const FixedArray<double, 3>& camera_position,
    const TransformationMatrix<float, double, 3>& object_model_matrix,
    const AxisAlignedBoundingBox<float, 3>& object_aabb);

}
