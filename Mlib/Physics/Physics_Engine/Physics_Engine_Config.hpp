#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Containers/Collision_Group.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Phase.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cmath>

namespace Mlib {

struct PhysicsEngineConfig {
    inline float dt_substeps(const PhysicsPhase& phase) const {
        return dt_substeps_(phase.group.nsubsteps);
    }
    inline float ncached(const PhysicsPhase& phase) const {
        return ncached_(phase.group.nsubsteps);
    }
    inline float dt_substeps_(size_t nsubsteps2) const {
        return dt / (float)nsubsteps2;
    }
    inline float ncached_(size_t nsubsteps2) const {
        return dt_io / dt_substeps_(nsubsteps2);
    }

    float dt = 0.01667f * seconds;
    float dt_io = 0.01667f * seconds;
    float max_residual_time = 0.5f * seconds;
    bool control_fps = true;
    bool print_residual_time = false;

    // BVH
    CompressedScenePos static_radius = (CompressedScenePos)(20.f * meters);
    CompressedScenePos bvh_max_size = (CompressedScenePos)(2.f * meters);
    size_t bvh_levels = 15;
    CompressedScenePos supply_depot_attraction_radius = (CompressedScenePos)(10.f * meters);

    // Grid
    size_t grid_level = 9;
    FixedArray<size_t, 3> ncells = { 400u, 5u, 400u };
    CompressedScenePos dilation_radius = (CompressedScenePos)(20.f * meters);

    // Collision/Friction misc.
    float max_extra_friction = 0;
    float max_extra_w = 0;
    float velocity_lambda_min = -1000.f * kph;
    float max_aerodynamic_acceleration = 50.f * kph / seconds;
    bool avoid_burnout = true;
    bool no_slip = false;
    float hand_brake_velocity = 2.f * kph;
    float max_penetration = 40.f * cm;
    float max_velocity_increase = 30.f * percent;

    // Friction
    // From: http://ffden-2.phys.uaf.edu/211_fall2002.web.dir/ben_townsend/staticandkineticfriction.htm
    float stiction_coefficient = 2;
    float friction_coefficient = 1.6f;
    float longitudinal_friction_steepness = 5;
    float lateral_friction_steepness = 7;  // 1 / sin(4 / 180 * pi) = 14.336

    // Collision
    float wheel_penetration_depth = 0.25f * meters;  // (penetration depth) + (shock absorber) = 0.2
    float max_keep_normal = 1.f * meters;
    // float overlap_clipped = 0.01f * meters;
    float max_min_cos_ridge = 1e-4f;
    float min_cos_ridge_triangle = 0.5f;
    float max_cos_round_normal = 0.8f;
    float min_skip_velocity = 1 * kph;
    float slide_factor = 2.f;
    float ignore_factor = 16.f;
    float intersection_point_radius = 0.03f * meters;
    float point_equality_beta = 0.15f;
    float plane_equality_beta = 0.15f;
    float plane_inequality_beta = 0.02f;
    size_t nsubsteps = 8;
    bool enable_ridge_map = false;  // disabled to save memory, the swept sphere volume is used instead.

    // Grind
    float max_grind_cos = 0.5;
    size_t nframes_straight_grind = 30;
    float continuos_grind_velocity_threshold = 0.1f * meters / seconds;
    float continuos_grind_cos_threshold = 0.5f;
    float grind_jump_dv = 5.f * meters / seconds;

    // Alignment
    float alignment_surface_cos = float(1e-6);
    float alignment_surface_cos_strict = 0.85f;
    float alignment_plane_cos = 0.7f;
    float alignment_slerp = 0.1f;
};

}
