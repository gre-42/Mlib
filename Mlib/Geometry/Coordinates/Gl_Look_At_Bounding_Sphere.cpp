#include "Gl_Look_At_Bounding_Sphere.hpp"
#include <Mlib/Geometry/Cameras/Distance_For_Fov.hpp>
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>

using namespace Mlib;

std::optional<GlLookatBoundingSphere> Mlib::gl_lookat_bounding_sphere(
    float fov,
    const TransformationMatrix<float, ScenePos, 3>& observer_model_matrix,
    const BoundingSphere<ScenePos, 3>& observed_bounding_sphere)
{
    if (observed_bounding_sphere.radius < ScenePos(1e-6)) {
        return std::nullopt;
    }
    GlLookatBoundingSphere result{
        .camera_model_matrix = uninitialized
    };
    auto dir = observed_bounding_sphere.center - observer_model_matrix.t;
    auto l2 = sum(squared(dir));
    if (l2 < 1e-12) {
        return std::nullopt;
    }
    dir /= std::sqrt(l2);
    auto dist_camera_to_observed = distance_for_fov((ScenePos)fov, observed_bounding_sphere.radius);
    result.camera_model_matrix.t = observed_bounding_sphere.center - dir * dist_camera_to_observed;
    auto lookat = gl_lookat_relative(dir.casted<float>());
    if (!lookat.has_value()) {
        return std::nullopt;
    }
    result.camera_model_matrix.R = *lookat;
    result.near_plane = (float)(dist_camera_to_observed - observed_bounding_sphere.radius);
    result.far_plane = (float)(dist_camera_to_observed + observed_bounding_sphere.radius);
    return result;
}
