#include "Projection_Matrix_Camera.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/linmath.hpp>

using namespace Mlib;

ProjectionMatrixCamera::ProjectionMatrixCamera(const FixedArray<float, 4, 4>& projection_matrix)
: projection_matrix_{projection_matrix}
{}

ProjectionMatrixCamera::~ProjectionMatrixCamera()
{}

std::shared_ptr<Camera> ProjectionMatrixCamera::copy() const {
    return std::make_shared<ProjectionMatrixCamera>(*this);
}

FixedArray<float, 4, 4> ProjectionMatrixCamera::projection_matrix() {
    return projection_matrix_;
}
