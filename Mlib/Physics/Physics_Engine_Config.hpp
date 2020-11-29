#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Collision/Resolve_Collision_Type.hpp>
#include <Mlib/Physics/Physics_Type.hpp>
#include <cmath>

namespace Mlib {

struct PhysicsEngineConfig {
    float dt = 0.01667;
    float max_residual_time = 0.5;
    bool print_residual_time = false;
    bool sat = true;
    bool collide_only_normals = false;
    float min_acceleration = 2;
    float min_velocity = 1e-1;
    float min_angular_velocity = 1e-2;
    float damping = 0; //std::exp(-7);
    float friction = 0; // std::exp(-8.5);
    float overlap_tolerance = 1.2;
    float hand_break_velocity = 0.5;
    // From: http://ffden-2.phys.uaf.edu/211_fall2002.web.dir/ben_townsend/staticandkineticfriction.htm
    float stiction_coefficient = 2;
    float friction_coefficient = 1.6;
    float alpha0 = 0.1;
    bool avoid_burnout = true;
    float lateral_stability = 1;
    float max_extra_friction = 0;
    float max_extra_w = 0;
    float longitudinal_friction_steepness = 5;
    float lateral_friction_steepness = 7;  // 1 / sin(4 / 180 * pi) = 14.336
    float wheel_penetration_depth = 0.25;  // (penetration depth) + (shock absorber) = 0.2
    float static_radius = 200;
    Interp<float> outness_fac_interp{{-0.5, 1}, {2000, 0}, OutOfRangeBehavior::CLAMP};
    PhysicsType physics_type = PhysicsType::VERSION1;
    ResolveCollisionType resolve_collision_type = ResolveCollisionType::PENALTY;
    float lambda_min = -10;
    float contact_beta = 0.5;
    float contact_beta2 = 0.2;
    bool bvh = true;
    size_t oversampling = 20;
};

}
