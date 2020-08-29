#include "Generic_Camera.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/linmath.hpp>

using namespace Mlib;

GenericCamera::GenericCamera(const CameraConfig& cfg, const Mode& mode)
: cfg_{cfg},
  requires_postprocessing_{true},
  mode_{mode}
{}

void GenericCamera::set_mode(const Mode& mode) {
    mode_ = mode;
}

GenericCamera::~GenericCamera()
{}

std::shared_ptr<Camera> GenericCamera::copy() const {
    return std::make_shared<GenericCamera>(*this);
}

void GenericCamera::set_y_fov(float y_fov) {
    cfg_.y_fov = y_fov;
}

void GenericCamera::set_aspect_ratio(float aspect_ratio) {
    cfg_.aspect_ratio = aspect_ratio;
}

void GenericCamera::set_near_plane(float near_plane) {
    cfg_.near_plane = near_plane;
}

void GenericCamera::set_far_plane(float far_plane) {
    cfg_.far_plane = far_plane;
}

void GenericCamera::set_left_plane(float left_plane) {
    cfg_.left_plane = left_plane;
}

void GenericCamera::set_right_plane(float right_plane) {
    cfg_.right_plane = right_plane;
}

void GenericCamera::set_bottom_plane(float bottom_plane) {
    cfg_.bottom_plane = bottom_plane;
}

void GenericCamera::set_top_plane(float top_plane) {
    cfg_.top_plane = top_plane;
}

float GenericCamera::get_near_plane() const {
    return cfg_.near_plane;
}

float GenericCamera::get_far_plane() const {
    return cfg_.far_plane;
}

void GenericCamera::set_requires_postprocessing(bool value) {
    requires_postprocessing_ = value;
}

bool GenericCamera::get_requires_postprocessing() const {
    return requires_postprocessing_;
}

FixedArray<float, 4, 4> GenericCamera::projection_matrix() {
    mat4x4 p;
    switch (mode_) {
        case Mode::ORTHO:
            mat4x4_ortho(p, cfg_.left_plane, cfg_.right_plane, cfg_.bottom_plane, cfg_.top_plane, cfg_.near_plane, cfg_.far_plane);
            break;
        case Mode::PERSPECTIVE:
            mat4x4_perspective(p, cfg_.y_fov, cfg_.aspect_ratio, cfg_.near_plane, cfg_.far_plane);
            break;
        default:
            throw std::runtime_error("Unknown camera mode");
    }
    //mat4x4_frustum(p, -ratio / 10, ratio / 10, -1.f / 10, 1.f / 10, 2.f, 10.f);

    assert(sizeof(p) == sizeof(FixedArray<float, 4, 4>));
    return reinterpret_cast<FixedArray<float, 4, 4>*>(&p)->T();
}
