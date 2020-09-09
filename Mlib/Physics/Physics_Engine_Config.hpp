#pragma once
#include <Mlib/Math/Interp.hpp>
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
    float damping = std::exp(-7);
    float friction = std::exp(-8.5);
    float overlap_tolerance = 1.2;
    float break_accel = 5;
    float tangential_accel = 10;
    float hand_break_velocity = 0.5;
    // From: http://ffden-2.phys.uaf.edu/211_fall2002.web.dir/ben_townsend/staticandkineticfriction.htm
    float stiction_coefficient = 2;
    float friction_coefficient = 1.6;
    bool avoid_burnout = true;
    float wheel_penetration_depth = 0.15;  // (penetration depth) + (shock absorber) = 0.2
    float static_radius = 200;
    Interp<float> outness_fac_interp{{-0.5, 1}, {200, 0}, false, 200, 0};
};

}
