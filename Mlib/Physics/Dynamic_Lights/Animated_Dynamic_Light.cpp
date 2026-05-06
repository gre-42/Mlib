#include "Animated_Dynamic_Light.hpp"
#include <Mlib/Physics/Dynamic_Lights/Dynamic_Lights.hpp>
#include <Mlib/Physics/Units.hpp>

using namespace Mlib;

AnimatedDynamicLight::AnimatedDynamicLight(
    std::function<FixedArray<ScenePos, 3>()> get_position,
    std::chrono::steady_clock::time_point time,
    const AnimatedDynamicLightConfiguration& config,
    DynamicLights& container)
    : creation_time_{ time }
    , get_position_{ std::move(get_position) }
    , position_history_ { get_position_(), time }
    , config_{ config }
    , container_{ container }
    , position_{ fixed_nans<ScenePos, 3>() }
    , color_{ fixed_nans<float, 3>() }
{}

AnimatedDynamicLight::~AnimatedDynamicLight() {
    container_.erase(*this);
}

void AnimatedDynamicLight::append_time(std::chrono::steady_clock::time_point time)
{
    position_history_.append(get_position_(), time);
}

void AnimatedDynamicLight::set_time(std::chrono::steady_clock::time_point time) {
    position_ = position_history_.get(time);
    color_ = config_.time_to_color(elapsed(time));
}

FixedArray<float, 3> AnimatedDynamicLight::get_color(const FixedArray<ScenePos, 3>& target_position) const
{
    auto dist2 = sum(squared(position_ - target_position));
    return config_.squared_distance_to_intensity(dist2) * color_;
}

bool AnimatedDynamicLight::animation_completed(std::chrono::steady_clock::time_point time) const
{
    return elapsed(time) > config_.time_to_color.xmax();
}

float AnimatedDynamicLight::elapsed(std::chrono::steady_clock::time_point time) const {
    return std::chrono::duration<float>(time - creation_time_).count() * seconds;
}
