#include "Engine_Exhaust.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>

using namespace Mlib;

EngineExhaust::EngineExhaust(
    RenderingResources& rendering_resources,
    SceneNodeResources& scene_node_resources,
    std::shared_ptr<IParticleRenderer> particle_renderer,
    Scene& scene,
    const ConstantParticleTrail& particle,
    const TransformationMatrix<SceneDir, ScenePos, 3>& relative_location,
    float p_reference)
    : smoke_generator_{
        rendering_resources,
        scene_node_resources,
        particle_renderer,
        scene }
    , trail_generator_{ smoke_generator_ }
    , particle_{ particle }
    , relative_location_{ relative_location }
    , p_reference_{ p_reference }
{}

EngineExhaust::~EngineExhaust() = default;

void EngineExhaust::notify_rotation(
    float engine_angular_velocity,
    float tires_angular_velocity,
    const EnginePowerIntent& engine_power_intent,
    float max_surface_power,
    const StaticWorld& static_world)
{
    float p = engine_power_intent.real_power(
        engine_angular_velocity,
        tires_angular_velocity,
        max_surface_power);
    if ((p >= p_reference_) && maybe_generate_(particle_.generation_dt)) {
        trail_generator_.generate(
            relative_location_.t,
            matrix_2_tait_bryan_angles(relative_location_.R),
            fixed_zeros<SceneDir, 3>(),     // velocity
            0.f,                            // texture_layer
            particle_.particle,
            "exhaust",
            ParticleContainer::INSTANCE,
            static_world);
    }
}

void EngineExhaust::set_location(
    const RotatingFrame<SceneDir, ScenePos, 3>& frame)
{}

void EngineExhaust::advance_time(float dt) {
    maybe_generate_.advance_time(dt);
}
