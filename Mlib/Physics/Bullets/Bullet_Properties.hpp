#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <string>

namespace Mlib {

enum class RigidBodyVehicleFlags;

struct BulletProperties {
    std::string renderable_resource_name;
    std::string hitbox_resource_name;
    std::string explosion_resource_name;
    float explosion_animation_time;
    RigidBodyVehicleFlags rigid_body_flags;
    float mass;
    float velocity;
    float max_lifetime;
    float damage;
    float damage_radius;
    FixedArray<float, 3> size;
    std::string trail_resource_name;
    float trail_dt;
    float trail_animation_duration;
    std::string trace_storage;
    std::string dynamic_light_configuration_before_impact;
    std::string dynamic_light_configuration_after_impact;
};

}
