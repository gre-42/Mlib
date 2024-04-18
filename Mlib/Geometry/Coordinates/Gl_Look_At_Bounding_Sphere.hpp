#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

namespace Mlib {

template <class TData, size_t tndim>
class BoundingSphere;

struct GlLookatBoundingSphere {
    TransformationMatrix<float, double, 3> camera_model_matrix;
    float near_plane;
    float far_plane;
};

std::optional<GlLookatBoundingSphere> gl_lookat_bounding_sphere(
    float fov,
    const TransformationMatrix<float, double, 3>& observer_model_matrix,
    const BoundingSphere<double, 3>& observed_bounding_sphere);

}
