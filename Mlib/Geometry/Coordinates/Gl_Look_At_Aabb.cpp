#include "Gl_Look_At_Aabb.hpp"
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>

using namespace Mlib;

std::optional<GlLookatAabb> Mlib::gl_lookat_aabb(
    float dpi,
    const FixedArray<double, 3>& camera_position,
    const TransformationMatrix<float, double, 3>& object_model_matrix,
    const AxisAlignedBoundingBox<float, 3>& object_aabb)
{
    GlLookatAabb result;
    auto d = FixedArray<float, 2>{
        float(object_model_matrix.t()(0) - camera_position(0)),
        float(object_model_matrix.t()(2) - camera_position(2))};
    auto d_len = std::sqrt(sum(squared(d)));
    if (d_len < 1e-12) {
        return std::nullopt;
    }
    d /= d_len;
    result.extrinsic_R = gl_lookat_relative(FixedArray<float, 3>{d(0), 0.f, d(1)});
    TransformationMatrix<float, double, 3> lookat0{
        result.extrinsic_R,
        camera_position};
    auto object_model_matrix_rel = (lookat0.inverted() * object_model_matrix).casted<float, float>();
    AxisAlignedBoundingBox<float, 2> frustum_aabb;
    result.frustum_camera_config.near_plane = INFINITY;
    result.frustum_camera_config.far_plane = -INFINITY;
    if (!object_aabb.for_each_corner([&](const FixedArray<float, 3>& corner) {
        auto dc = object_model_matrix_rel.transform(corner);
        // This also excludes points behind the camera.
        if (dc(2) > -1e-12) {
            return false;
        }
        frustum_aabb.extend(FixedArray<float, 2>{dc(0), dc(1)} / dc(2));
        result.frustum_camera_config.near_plane = std::min(result.frustum_camera_config.near_plane, -dc(2));
        result.frustum_camera_config.far_plane = std::max(result.frustum_camera_config.far_plane, -dc(2));
        return true;
    }))
    {
        return std::nullopt;
    }
    result.frustum_camera_config.left = frustum_aabb.min()(0) * result.frustum_camera_config.near_plane;
    result.frustum_camera_config.right = frustum_aabb.max()(0) * result.frustum_camera_config.near_plane;
    result.frustum_camera_config.bottom = frustum_aabb.min()(1) * result.frustum_camera_config.near_plane;
    result.frustum_camera_config.top = frustum_aabb.max()(1) * result.frustum_camera_config.near_plane;
    return result;
}
