#include "Engine_Exhaust.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>

using namespace Mlib;

EngineExhaust::EngineExhaust(
    SmokeParticleGenerator& smoke_generator,
    const ConstantParticleTrail& particle,
    const TransformationMatrix<SceneDir, ScenePos, 3>& relative_location)
    : smoke_generator_{ smoke_generator }
    , particle_{ particle }
    , relative_location_{ relative_location }
{}

EngineExhaust::~EngineExhaust() = default;

void EngineExhaust::notify_rotation(
    float engine_angular_velocity,
    float tires_angular_velocity,
    const EnginePowerIntent& engine_power_intent,
    float max_surface_power)
{}

void EngineExhaust::set_location(
    const RotatingFrame<SceneDir, ScenePos, 3>& frame)
{
    parent_frame_.emplace(frame);
}

void EngineExhaust::advance_time(float dt) {
    if (!parent_frame_.has_value()) {
        return;
    }
    auto location = parent_frame_->location * relative_location_;
    auto velocity = parent_frame_->velocity_at_position(location.t);
    smoke_generator_.generate_root(
        particle_.particle.resource_name,
        VariableAndHash<std::string>{"exhaust" + smoke_generator_.generate_suffix()},
        location.t,
        matrix_2_tait_bryan_angles(location.R),
        velocity,
        particle_.particle.air_resistance,
        particle_.particle.animation_duration,
        ParticleType::INSTANCE);
}
