#include "Constant_Dynamic_Light.hpp"
#include <Mlib/Physics/Dynamic_Lights/Dynamic_Lights.hpp>

using namespace Mlib;

ConstantDynamicLight::ConstantDynamicLight(
    std::function<FixedArray<ScenePos, 3>()> get_position,
    std::chrono::steady_clock::time_point time,
    const ConstantDynamicLightConfiguration& config,
    DynamicLights& container)
    : get_position_{ std::move(get_position) }
    , position_{ get_position_() }
    , position_history_ { position_, time }
    , config_{ config }
    , container_{ container }
{}

ConstantDynamicLight::~ConstantDynamicLight() {
    container_.erase(*this);
}

void ConstantDynamicLight::append_time(std::chrono::steady_clock::time_point time)
{
    position_history_.append(get_position_(), time);
}

void ConstantDynamicLight::set_time(std::chrono::steady_clock::time_point time)
{
    position_ = position_history_.get(time);
}

FixedArray<float, 3> ConstantDynamicLight::get_color(const FixedArray<ScenePos, 3>& target_position) const
{
    auto dist2 = sum(squared(position_ - target_position));
    return config_.squared_distance_to_intensity(dist2) * config_.color;
}

bool ConstantDynamicLight::animation_completed(std::chrono::steady_clock::time_point time) const
{
    return false;
}
