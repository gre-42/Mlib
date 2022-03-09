#include "Camera.hpp"
#include <stdexcept>

using namespace Mlib;

Camera::Camera() {}

Camera::~Camera() {}

void Camera::set_y_fov(float y_fov) {}

void Camera::set_aspect_ratio(float aspect_ratio) {}

void Camera::set_near_plane(float near_plane) {}

void Camera::set_far_plane(float far_plane) {}

void Camera::set_left_plane(float left_plane) {}

void Camera::set_right_plane(float right_plane) {}

void Camera::set_bottom_plane(float bottom_plane) {}

void Camera::set_top_plane(float top_plane) {}

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
