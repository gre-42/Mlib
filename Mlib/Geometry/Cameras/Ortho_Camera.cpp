#include "Ortho_Camera.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/linmath.hpp>
#include <mutex>

using namespace Mlib;

OrthoCamera::OrthoCamera(
    const OrthoCameraConfig& cfg,
    Postprocessing postprocessing)
: cfg_{cfg},
  postprocessing_{postprocessing}
{}

OrthoCamera::~OrthoCamera()
{}

std::unique_ptr<Camera> OrthoCamera::copy() const {
    std::shared_lock lock{mutex_};
    return std::make_unique<OrthoCamera>(cfg_, postprocessing_);
}

void OrthoCamera::set_near_plane(float near_plane) {
    std::unique_lock lock{mutex_};
    cfg_.near_plane = near_plane;
}

void OrthoCamera::set_far_plane(float far_plane) {
    std::unique_lock lock{mutex_};
    cfg_.far_plane = far_plane;
}

void OrthoCamera::set_left_plane(float left_plane) {
    std::unique_lock lock{mutex_};
    cfg_.left_plane = left_plane;
}

void OrthoCamera::set_right_plane(float right_plane) {
    std::unique_lock lock{mutex_};
    cfg_.right_plane = right_plane;
}

void OrthoCamera::set_bottom_plane(float bottom_plane) {
    std::unique_lock lock{mutex_};
    cfg_.bottom_plane = bottom_plane;
}

void OrthoCamera::set_top_plane(float top_plane) {
    std::unique_lock lock{mutex_};
    cfg_.top_plane = top_plane;
}

float OrthoCamera::get_near_plane() const {
    std::shared_lock lock{mutex_};
    return cfg_.near_plane;
}

float OrthoCamera::get_far_plane() const {
    std::shared_lock lock{mutex_};
    return cfg_.far_plane;
}

void OrthoCamera::set_requires_postprocessing(bool value) {
    std::unique_lock lock{mutex_};
    postprocessing_ = (Postprocessing)value;
}

bool OrthoCamera::get_requires_postprocessing() const {
    std::shared_lock lock{mutex_};
    return (bool)postprocessing_;
}

FixedArray<float, 4, 4> OrthoCamera::projection_matrix() {
    std::shared_lock lock{mutex_};
    mat4x4 p;
    mat4x4_ortho(p, cfg_.left_plane, cfg_.right_plane, cfg_.bottom_plane, cfg_.top_plane, cfg_.near_plane, cfg_.far_plane);
    //mat4x4_frustum(p, -ratio / 10, ratio / 10, -1.f / 10, 1.f / 10, 2.f, 10.f);
    static_assert(sizeof(p) == sizeof(FixedArray<float, 4, 4>));
    return reinterpret_cast<FixedArray<float, 4, 4>*>(&p)->T();
}