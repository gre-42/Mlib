#include "Frustum_Camera.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/linmath.hpp>
#include <mutex>

using namespace Mlib;

FrustumCamera::FrustumCamera(
    const FrustumCameraConfig& cfg,
    Postprocessing postprocessing)
    : cfg_{ cfg }
    , postprocessing_{ postprocessing }
{}

FrustumCamera::~FrustumCamera() = default;

std::unique_ptr<Camera> FrustumCamera::copy() const {
    std::shared_lock lock{ mutex_ };
    return std::make_unique<FrustumCamera>(cfg_, postprocessing_);
}

void FrustumCamera::set_near_plane(float near_plane) {
    std::scoped_lock lock{ mutex_ };
    cfg_.near_plane = near_plane;
}

void FrustumCamera::set_far_plane(float far_plane) {
    std::scoped_lock lock{ mutex_ };
    cfg_.far_plane = far_plane;
}

void FrustumCamera::set_left(float left) {
    std::scoped_lock lock{ mutex_ };
    cfg_.left = left;
}

void FrustumCamera::set_right(float right) {
    std::scoped_lock lock{ mutex_ };
    cfg_.right = right;
}

void FrustumCamera::set_bottom(float bottom) {
    std::scoped_lock lock{ mutex_ };
    cfg_.bottom = bottom;
}

void FrustumCamera::set_top(float top) {
    std::scoped_lock lock{ mutex_ };
    cfg_.top = top;
}

float FrustumCamera::get_near_plane() const {
    std::shared_lock lock{ mutex_ };
    return cfg_.near_plane;
}

float FrustumCamera::get_far_plane() const {
    std::shared_lock lock{ mutex_ };
    return cfg_.far_plane;
}

void FrustumCamera::set_requires_postprocessing(bool value) {
    std::scoped_lock lock{ mutex_ };
    postprocessing_ = (Postprocessing)value;
}

bool FrustumCamera::get_requires_postprocessing() const {
    std::shared_lock lock{ mutex_ };
    return (bool)postprocessing_;
}

FixedArray<float, 4, 4> FrustumCamera::projection_matrix() const {
    std::shared_lock lock{ mutex_ };
    mat4x4 p;
    mat4x4_frustum(p, cfg_.left, cfg_.right, cfg_.bottom, cfg_.top, cfg_.near_plane, cfg_.far_plane);
    static_assert(sizeof(p) == sizeof(FixedArray<float, 4, 4>));
    return reinterpret_cast<FixedArray<float, 4, 4>*>(&p)->T();
}
