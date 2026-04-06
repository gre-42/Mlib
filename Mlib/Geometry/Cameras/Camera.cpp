
#include "Camera.hpp"
#include <stdexcept>

using namespace Mlib;

Camera::Camera() = default;

Camera::~Camera() = default;

void Camera::set_aspect_ratio(float aspect_ratio) {}

float Camera::get_near_plane() const {
    throw std::runtime_error("get_near_plane not implemented");
}

float Camera::get_far_plane() const {
    throw std::runtime_error("get_far_plane not implemented");
}

void Camera::set_requires_postprocessing(bool value) {
    throw std::runtime_error("set_requires_postprocessing not implemented");
}

bool Camera::get_requires_postprocessing() const {
    throw std::runtime_error("get_requires_postprocessing not implemented");
}
