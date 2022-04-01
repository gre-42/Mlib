#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Units.hpp>
#include <cmath>

namespace Mlib {

struct PhysicsEngineConfig {
    float dt = 0.01667f * s;
    float max_residual_time = 0.5f * s;
    bool print_residual_time = false;
    bool sat = true;
    bool collide_only_normals = false;
    float min_acceleration = 2 * meters / (s * s);
    float min_velocity = (float)1e-1 * meters / s;
    float min_angular_velocity = (float)1e-2 * radians / s;
    float damping = 0; //std::exp(-7);
    float friction = 0; // std::exp(-8.5);
    float overlap_tolerance = (float)1.2;
    float hand_brake_velocity = 0.5f * meters / s;
    // From: http://ffden-2.phys.uaf.edu/211_fall2002.web.dir/ben_townsend/staticandkineticfriction.htm
    float stiction_coefficient = 2;
    float friction_coefficient = 1.6f;
    float max_grind_cos = 0.5;
    float alpha0 = 0.1f;
    bool avoid_burnout = true;
    bool no_slip = false;
    float lateral_stability = 1;
    float max_extra_friction = 0;
    float max_extra_w = 0;
    float longitudinal_friction_steepness = 5;
    float lateral_friction_steepness = 7;  // 1 / sin(4 / 180 * pi) = 14.336
    float wheel_penetration_depth = 0.25f;  // (penetration depth) + (shock absorber) = 0.2
    float static_radius = 200;
    float bvh_max_size = 50;
    Interp<float> outness_fac_interp{{-0.5f, 1.f}, {2000.f, 0.f}, OutOfRangeBehavior::CLAMP};
    float velocity_lambda_min = -1000.f * kph;
    float point_equality_beta = 0.05f;
    float plane_equality_beta = 0.05f;
    float plane_inequality_beta = 0.02f;
    size_t oversampling = 20;

    // Grind
    size_t nframes_straight_grind = 30;
    float continuos_grind_velocity_threshold = 0.1f;
    float continuos_grind_projected_velocity_threshold = 0.5f;
    float grind_jump_dv = 5.f * meters / s;

    // Alignment
    float alignment_surface_cos = float{1e-6};
    float alignment_surface_cos_strict = 0.85f;
    float alignment_plane_cos = 0.7f;
    float alignment_slerp = 0.1f;
};

}
