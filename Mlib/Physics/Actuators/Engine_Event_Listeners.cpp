#include "Engine_Event_Listeners.hpp"

using namespace Mlib;

EngineEventListeners::EngineEventListeners() = default;

EngineEventListeners::~EngineEventListeners() = default;

void EngineEventListeners::notify_rotation(
    float engine_angular_velocity,
    float tires_angular_velocity,
    const EnginePowerIntent& engine_power_intent,
    float max_surface_power)
{
    for (const auto& l : listeners_) {
        l->notify_rotation(
            engine_angular_velocity,
            tires_angular_velocity,
            engine_power_intent,
            max_surface_power);
    }
}

void EngineEventListeners::set_location(const RotatingFrame<SceneDir, ScenePos, 3>& frame)
{
    for (const auto& l : listeners_) {
        l->set_location(frame);
    }
}

void EngineEventListeners::advance_time(float dt) {
    for (const auto& l : listeners_) {
        l->advance_time(dt);
    }
}

void EngineEventListeners::add(std::shared_ptr<IEngineEventListener> l) {
    listeners_.emplace_back(std::move(l));
}
