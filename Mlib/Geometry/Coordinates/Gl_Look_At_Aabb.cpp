#include "Gl_Look_At_Aabb.hpp"
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>

using namespace Mlib;

std::optional<GlLookatAabb> Mlib::gl_lookat_aabb(
    const FixedArray<ScenePos, 3>& camera_position,
    const TransformationMatrix<float, ScenePos, 3>& object_model_matrix,
    const AxisAlignedBoundingBox<ScenePos, 3>& object_aabb)
{
    GlLookatAabb result{
        .extrinsic_R = uninitialized,
        .sensor_aabb = AxisAlignedBoundingBox<float, 2>::empty()
    };
    auto d = FixedArray<float, 2>{
        float(object_model_matrix.t(0) - camera_position(0)),
        float(object_model_matrix.t(2) - camera_position(2))};
    auto d_len = std::sqrt(sum(squared(d)));
    if (d_len < 1e-12) {
        return std::nullopt;
    }
    d /= d_len;
    auto R = gl_lookat_relative(FixedArray<float, 3>{d(0), 0.f, d(1)});
    if (!R.has_value()) {
        return std::nullopt;
    }
    result.extrinsic_R = *R;
    TransformationMatrix<float, ScenePos, 3> lookat0{
        result.extrinsic_R,
        camera_position};
    auto object_model_matrix_rel = lookat0.inverted() * object_model_matrix;
    result.near_plane = INFINITY;
    result.far_plane = -INFINITY;
    if (!object_aabb.for_each_corner([&](const FixedArray<ScenePos, 3>& corner) {
        auto dc = object_model_matrix_rel.transform(corner).casted<float>();
        // This also excludes points behind the camera.
        if (dc(2) > -1e-12) {
            return false;
        }
        result.sensor_aabb.extend(FixedArray<float, 2>{dc(0), dc(1)} / (-dc(2)));
        result.near_plane = std::min(result.near_plane, -dc(2));
        result.far_plane = std::max(result.far_plane, -dc(2));
        return true;
    }))
    {
        return std::nullopt;
    }
    return result;
}
