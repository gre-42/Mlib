#include "Camera.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

using namespace Mlib;

Camera::Camera() = default;

Camera::~Camera() = default;

void Camera::set_aspect_ratio(float aspect_ratio) {}

float Camera::get_near_plane() const {
    THROW_OR_ABORT("get_near_plane not implemented");
}

float Camera::get_far_plane() const {
    THROW_OR_ABORT("get_far_plane not implemented");
}

void Camera::set_requires_postprocessing(bool value) {
    THROW_OR_ABORT("set_requires_postprocessing not implemented");
}

bool Camera::get_requires_postprocessing() const {
    THROW_OR_ABORT("get_requires_postprocessing not implemented");
}
