#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Smoke_Generation/Constant_Particle_Trail.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <string>

namespace Mlib {

enum class RigidBodyVehicleFlags;

struct BulletProperties {
    VariableAndHash<std::string> renderable_resource_name;
    VariableAndHash<std::string> hitbox_resource_name;
    VariableAndHash<std::string> explosion_resource_name;
    float explosion_animation_time;
    VariableAndHash<std::string> explosion_audio_resource_name;
    RigidBodyVehicleFlags rigid_body_flags;
    float mass;
    float velocity;
    float max_lifetime;
    float damage;
    float damage_radius;
    FixedArray<float, 3> size = uninitialized;
    ConstantParticleTrail trail;
    VariableAndHash<std::string> trace_storage;
    std::string dynamic_light_configuration_before_impact;
    std::string dynamic_light_configuration_after_impact;
};

}
