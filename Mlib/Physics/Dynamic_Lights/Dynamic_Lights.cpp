#include "Dynamic_Lights.hpp"
#include <Mlib/Physics/Dynamic_Lights/Animated_Dynamic_Light.hpp>
#include <Mlib/Physics/Dynamic_Lights/Constant_Dynamic_Light.hpp>
#include <Mlib/Physics/Dynamic_Lights/Dynamic_Light_Db.hpp>
#include <Mlib/Physics/Units.hpp>

using namespace Mlib;

DynamicLights::DynamicLights(const DynamicLightDb& db)
    : db_{ db }
{}

DynamicLights::~DynamicLights() = default;

std::unique_ptr<IDynamicLight> DynamicLights::instantiate(
    const std::string& name,
    const std::function<FixedArray<ScenePos, 3>()>& get_position,
    std::chrono::steady_clock::time_point time)
{
    const auto& config = db_.get(name);
    std::unique_ptr<IDynamicLight> result;
    if (std::holds_alternative<AnimatedDynamicLightConfiguration>(config)) {
        result = std::make_unique<AnimatedDynamicLight>(get_position, time, std::get<AnimatedDynamicLightConfiguration>(config), *this);
    } else if (std::holds_alternative<ConstantDynamicLightConfiguration>(config)) {
        result = std::make_unique<ConstantDynamicLight>(get_position, time, std::get<ConstantDynamicLightConfiguration>(config), *this);
    } else {
        THROW_OR_ABORT("Unknown dynamic light type");
    }
    std::scoped_lock lock{ mutex_ };
    if (!instances_.insert(result.get()).second) {
        verbose_abort("DynamicLights::instantiate internal error");
    }
    return result;
}

void DynamicLights::erase(IDynamicLight& light) {
    std::scoped_lock lock{ mutex_ };
    if (instances_.erase(&light) != 1) {
        verbose_abort("Could not delete dynamic light");
    }
}

void DynamicLights::append_time(std::chrono::steady_clock::time_point time) {
    std::scoped_lock lock{ mutex_ };
    for (const auto& l : instances_) {
        l->append_time(time);
    }
}

void DynamicLights::set_time(std::chrono::steady_clock::time_point time) {
    std::scoped_lock lock{ mutex_ };
    for (auto it = instances_.begin(); it != instances_.end(); ) {
        auto& l = **it;
        if (l.animation_completed(time)) {
            instances_.erase(it++);
        } else {
            l.set_time(time);
            ++it;
        }
    }
}

FixedArray<float, 3> DynamicLights::get_color(const FixedArray<ScenePos, 3>& target_position) const {
    FixedArray<float, 3> result = fixed_zeros<float, 3>();
    std::scoped_lock lock{ mutex_ };
    for (const auto& l : instances_) {
        result += l->get_color(target_position);
    }
    return result;
}
