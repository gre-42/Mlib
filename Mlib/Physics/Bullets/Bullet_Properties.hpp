#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Smoke_Generation/Constant_Particle_Trail.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;
enum class RigidBodyVehicleFlags;
enum class ParticleContainer;

struct BulletExplosion {
    std::optional<std::unordered_set<PhysicsMaterial>> materials;
    VariableAndHash<std::string> resource_name;
    ParticleContainer particle_container;
    float animation_time;
    VariableAndHash<std::string> audio_resource_name;
};

struct BulletProperties {
    VariableAndHash<std::string> renderable_resource_name;
    VariableAndHash<std::string> hitbox_resource_name;
    VariableAndHash<std::string> engine_audio_resource_name;
    std::vector<BulletExplosion> explosions;
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
