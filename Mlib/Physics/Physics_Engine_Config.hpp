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
    float max_stiction_force = 1e4;
    float max_friction_force = 2e3;
    bool avoid_burnout = true;
    float wheel_penetration_depth = 0.2;
    float static_radius = 200;
    Interp<float> outness_fac_interp{{-0.5, 1}, {200, 0}, false, 200, 0};
    Interp<float> contact_interp{{0.f, 0.05f}, {0.f, 1.f}, false, 0.f, 1.f};
};

}
