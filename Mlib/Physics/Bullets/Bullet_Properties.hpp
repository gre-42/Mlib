#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Interp.hpp>
#include <optional>
#include <string>

namespace Mlib {

enum class RigidBodyVehicleFlags;

struct BulletIllumination {
    float radius;
    Interp<float, FixedArray<float, 3>> colors = { {}, {} };
};

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
    std::optional<BulletIllumination> illumination;
};

}
