#include "Projection_Matrix_Camera.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <stdexcept>

using namespace Mlib;

ProjectionMatrixCamera::ProjectionMatrixCamera(const FixedArray<float, 4, 4>& projection_matrix)
    : projection_matrix_{ projection_matrix }
{}

ProjectionMatrixCamera::~ProjectionMatrixCamera() = default;

std::unique_ptr<Camera> ProjectionMatrixCamera::copy() const {
    return std::make_unique<ProjectionMatrixCamera>(projection_matrix_);
}

FixedArray<float, 4, 4> ProjectionMatrixCamera::projection_matrix() const {
    return projection_matrix_;
}

FixedArray<float, 2> ProjectionMatrixCamera::dpi(const FixedArray<float, 2>& texture_size) const {
    throw std::runtime_error("DPI computation not implemented for ProjectionMatrixCamera");
}
