#include "Generic_Camera.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/linmath.hpp>
#include <mutex>

using namespace Mlib;

GenericCamera::GenericCamera(
    const CameraConfig& cfg,
    Postprocessing postprocessing,
    Mode mode)
: cfg_{cfg},
  postprocessing_{postprocessing},
  mode_{mode}
{}

void GenericCamera::set_mode(const Mode& mode) {
    std::unique_lock lock{mutex_};
    mode_ = mode;
}

GenericCamera::~GenericCamera()
{}

std::unique_ptr<Camera> GenericCamera::copy() const {
    std::shared_lock lock{mutex_};
    return std::make_unique<GenericCamera>(cfg_, postprocessing_, mode_);
}

void GenericCamera::set_y_fov(float y_fov) {
    std::unique_lock lock{mutex_};
    cfg_.y_fov = y_fov;
}

void GenericCamera::set_aspect_ratio(float aspect_ratio) {
    std::unique_lock lock{mutex_};
    cfg_.aspect_ratio = aspect_ratio;
}

void GenericCamera::set_near_plane(float near_plane) {
    std::unique_lock lock{mutex_};
    cfg_.near_plane = near_plane;
}

void GenericCamera::set_far_plane(float far_plane) {
    std::unique_lock lock{mutex_};
    cfg_.far_plane = far_plane;
}

void GenericCamera::set_left_plane(float left_plane) {
    std::unique_lock lock{mutex_};
    cfg_.left_plane = left_plane;
}

void GenericCamera::set_right_plane(float right_plane) {
    std::unique_lock lock{mutex_};
    cfg_.right_plane = right_plane;
}

void GenericCamera::set_bottom_plane(float bottom_plane) {
    std::unique_lock lock{mutex_};
    cfg_.bottom_plane = bottom_plane;
}

void GenericCamera::set_top_plane(float top_plane) {
    std::unique_lock lock{mutex_};
    cfg_.top_plane = top_plane;
}

float GenericCamera::get_near_plane() const {
    std::shared_lock lock{mutex_};
    return cfg_.near_plane;
}

float GenericCamera::get_far_plane() const {
    std::shared_lock lock{mutex_};
    return cfg_.far_plane;
}

void GenericCamera::set_requires_postprocessing(bool value) {
    std::unique_lock lock{mutex_};
    postprocessing_ = (Postprocessing)value;
}

bool GenericCamera::get_requires_postprocessing() const {
    std::shared_lock lock{mutex_};
    return (bool)postprocessing_;
}

FixedArray<float, 4, 4> GenericCamera::projection_matrix() {
    std::shared_lock lock{mutex_};
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

    static_assert(sizeof(p) == sizeof(FixedArray<float, 4, 4>));
    return reinterpret_cast<FixedArray<float, 4, 4>*>(&p)->T();
}
