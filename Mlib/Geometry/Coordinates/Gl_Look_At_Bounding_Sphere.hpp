#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

template <class TPos, size_t tndim>
class BoundingSphere;

struct GlLookatBoundingSphere {
    TransformationMatrix<float, ScenePos, 3> camera_model_matrix;
    float near_plane;
    float far_plane;
};

std::optional<GlLookatBoundingSphere> gl_lookat_bounding_sphere(
    float fov,
    const TransformationMatrix<float, ScenePos, 3>& observer_model_matrix,
    const BoundingSphere<ScenePos, 3>& observed_bounding_sphere);

}
