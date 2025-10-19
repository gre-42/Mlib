#include "Rigid_Body_Vehicle.hpp"
#include <Mlib/Geography/Geographic_Coordinates.hpp>
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Coordinates/Rotate_Axis_Onto_Other_Axis.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Fixed_Scaled_Unit_Vector.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Ref.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Actuators/Base_Rotor.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Delta_Intent.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Delta_Engine.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Actuators/Rotor.hpp>
#include <Mlib/Physics/Actuators/Tire.hpp>
#include <Mlib/Physics/Actuators/Tire_Power_Intent.hpp>
#include <Mlib/Physics/Actuators/Wing.hpp>
#include <Mlib/Physics/Ai/Skill_Factor.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Resolve/Constraints.hpp>
#include <Mlib/Physics/Interfaces/ICollision_Normal_Modifier.hpp>
#include <Mlib/Physics/Interfaces/IDamageable.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Physics/Interfaces/ISpawner.hpp>
#include <Mlib/Physics/Interfaces/ISurface_Normal.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Time_Step.hpp>
#include <Mlib/Physics/Rigid_Body/Actor_Task.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Domain.hpp>
#include <Mlib/Physics/Rotating_Frame.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Avatar_Controllers/Rigid_Body_Avatar_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Missile_Controllers/Rigid_Body_Missile_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Plane_Controllers/Rigid_Body_Plane_Controller.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Extender.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <chrono>

enum class WheelConstraintType {
    PLANES,
    ROD
};

static const float WHEEL_RADIUS = 0.25f;
static const auto WHEEL_CONSTRAINT_TYPE = WheelConstraintType::ROD;

using namespace Mlib;

RigidBodyVehicle::RigidBodyVehicle(
    ObjectPool& object_pool,
    const RigidBodyPulses& rbp,
    std::string name,
    std::string asset_id,
    const TransformationMatrix<double, double, 3>* geographic_mapping)
    : destruction_observers{ *this }
    , on_clear_scene_node_{ nullptr, CURRENT_SOURCE_LOCATION }
    , scene_node_{ nullptr }
    , max_velocity_{ INFINITY }
    , tires_{ "Tire", [](size_t i) { return std::to_string(i); } }
    , rotors_{ "Rotor", [](size_t i) { return std::to_string(i); } }
    , wings_{ "Wing", [](size_t i) { return std::to_string(i); } }
    , engines_{ "Engine" }
    , delta_engines_{ "Delta engine" }
    , flags_{ RigidBodyVehicleFlags::NONE }
#ifdef COMPUTE_POWER
    , power_{ NAN }
    , energy_old_{ NAN }
#endif
    , tires_z_{ 0.f, 0.f, -1.f }
    , rbp_{ rbp }
    , name_{ std::move(name) }
    , asset_id_{ std::move(asset_id) }
    , damageable_{ nullptr }
    , door_distance_{ NAN }
    , animation_state_updater_{ nullptr }
    , avatar_controller_{ nullptr}
    , vehicle_controller_{ nullptr}
    , jump_state_{
        .wants_to_jump_ = false,
        .wants_to_jump_oversampled_ = false,
        .jumping_counter_ = SIZE_MAX }
    , grind_state_{
        .wants_to_grind_ = false,
        .wants_to_grind_counter_ = 0,
        .grind_direction_ = fixed_nans<float, 3>(),
        .grinding_ = false,
        .grind_pv_ = fixed_nans<float, 3>()}
    , align_to_surface_state_{
        .align_to_surface_relaxation_ = 0.f,
        .touches_alignment_plane_ = false,
        .surface_normal_{ NAN, NAN, NAN }}
    , revert_surface_power_state_{
        .revert_surface_power_threshold_= INFINITY,
        .revert_surface_power_ = false}
    , fly_forward_state_{
        .wants_to_fly_forward_factor_ = NAN }
    , geographic_mapping_{ geographic_mapping }
    , current_vehicle_domain_{ VehicleDomain::UNDEFINED }
    , next_vehicle_domain_{ VehicleDomain::UNDEFINED }
    , actor_task_{ ActorTask::UNDEFINED }
    , object_pool_{ object_pool }
    , damage_absorption_direction_{ fixed_zeros<float, 3>() }
{
    if (name_.empty()) {
        THROW_OR_ABORT("No name given for rigid body vehicle");
    }
    // lerr() << "Rigid body vehicle \"" << name_ << '"';
    // lerr() << "I [kg*m²]\n" << rbi.rbp_.I_ / (kg * squared(meters));
}

RigidBodyVehicle::~RigidBodyVehicle()
{
    on_destroy.clear();
    destruction_observers.clear();
    drivers_.clear();
    if (scene_ != nullptr) {
        scene_->delete_root_node(node_name_);
    }
    while (!passengers_.empty()) {
        object_pool_.remove(passengers_.begin()->first.get());
    }
}

void RigidBodyVehicle::reset_forces(const PhysicsPhase& phase) {
    if (!phase.group.rigid_bodies.contains(&rbp_)) {
        return;
    }
    if (is_deactivated_avatar()) {
        THROW_OR_ABORT("Attempt to reset forces of deactivated avatar");
    }
    for (auto& e : engines_) {
        e.second.reset_forces();
    }

    // Must be above the block below.
    // Order is not important, because nframes_grinded_
    // is not read if (wants_to_grind_ == false).
    if (grind_state_.wants_to_grind_) {
        ++grind_state_.wants_to_grind_counter_;
    } else {
        grind_state_.wants_to_grind_counter_ = 0;
    }
    if (!grind_state_.grinding_) {
        grind_state_.grind_direction_ = NAN;
    }
    if (jump_state_.jumping_counter_ != SIZE_MAX) {
        jump_state_.jumping_counter_ += phase.group.divider;
    }

    // Must be below the block above.
    if (phase.substep == 0) {
        jump_state_.wants_to_jump_ = false;
        if (!grind_state_.grinding_) {
            grind_state_.wants_to_grind_ = false;
        }
        fly_forward_state_.wants_to_fly_forward_factor_ = NAN;
    }
    jump_state_.wants_to_jump_oversampled_ = false;
    grind_state_.grinding_ = false;
    align_to_surface_state_.touches_alignment_plane_ = false;
    align_to_surface_state_.surface_normal_ = NAN;
    current_vehicle_domain_ = next_vehicle_domain_;
    next_vehicle_domain_ = VehicleDomain::AIR;
}

void RigidBodyVehicle::set_wants_to_jump() {
    jump_state_.wants_to_jump_ = true;
    jump_state_.wants_to_jump_oversampled_ = true;
    jump_state_.jumping_counter_ = 0;
}

void RigidBodyVehicle::set_jump_dv(float value) {
    jump_dv_ = value;
}

void RigidBodyVehicle::integrate_force(
    const VectorAtPosition<float, ScenePos, 3>& F,
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase)
{
    auto dt = cfg.dt_substeps(phase);
    rbp_.integrate_impulse({.vector = F.vector * dt, .position = F.position}, 0.f, dt, CURRENT_SOURCE_LOCATION);
    // if (float len = sum(squared(F.vector)); len > 1e-12) {
    //     auto location = TransformationMatrix<float, ScenePos, 3>::identity();
    //     location.t() = F.position;
    //     location.R() *= std::sqrt(sum(squared(F.vector))) / (N * 100'000.f);
    //     g_beacons.push_back(Beacon{ .location = location });
    // }
}

void RigidBodyVehicle::integrate_force(
    const VectorAtPosition<float, ScenePos, 3>& F,
    const FixedArray<float, 3>& n,
    float damping,
    float friction,
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase)
{
    if (is_deactivated_avatar()) {
        THROW_OR_ABORT("Attempt to integrate forces of deactivated avatar");
    }
    integrate_force(F, cfg, phase);
    if (damping != 0) {
        auto vn = n * dot0d(rbp_.v_com_, n);
        auto vt = rbp_.v_com_ - vn;
        rbp_.v_com_ = (1 - damping) * vn + vt * (1 - friction);
        rbp_.w_ *= 1 - damping;
    }
}

// Note that g_beacons is delayed by one frame.
// namespace Mlib { thread_local extern std::list<Beacon> g_beacons; }

void RigidBodyVehicle::collide_with_air(CollisionHistory& c)
{
    if (!c.phase.group.rigid_bodies.contains(&rbp_)) {
        return;
    }
    if (is_deactivated_avatar()) {
        THROW_OR_ABORT("Attempt to collide deactivated avatar with air");
    }
    for (auto& [rotor_id, rotor] : rotors_) {
        TirePowerIntent P = consume_rotor_surface_power(rotor_id);
        if (P.type == TirePowerIntentType::ACCELERATE) {
            auto abs_location = rotor->rotated_location(rbp_.abs_transformation(), rbp_.v_com_, c.world);
            // g_beacons.push_back(Beacon{ .location = abs_location, .resource_name = "flag_z" });
            integrate_force(
                VectorAtPosition<float, ScenePos, 3>{
                    .vector = z3_from_3x3(abs_location.R) * P.power * P.relaxation * rotor->power2lift,
                    .position = abs_location.t },
                c.cfg,
                c.phase);
        }
        set_rotor_angular_velocity(rotor_id, rotor->w, c.cfg, c.phase, P.power);
        if (rotor->rbp != nullptr) {
            rotor->rbp->w_ = rotor->angular_velocity * z3_from_3x3(rotor->rbp->rotation_);
            auto T0 = rbp_.abs_transformation();
            auto T1 = rotor->rbp->abs_transformation();
            c.contact_infos.push_back(std::make_unique<PointContactInfo2>(
                rbp_,
                *rotor->rbp,
                PointEqualityConstraint{
                    .p0 = T0.transform(rotor->vehicle_mount_0.casted<ScenePos>()),
                    .p1 = T1.transform(rotor->blades_mount_0.casted<ScenePos>()),
                    .beta = c.cfg.point_equality_beta}));
            c.contact_infos.push_back(std::make_unique<PointContactInfo2>(
                rbp_,
                *rotor->rbp,
                PointEqualityConstraint{
                    .p0 = T0.transform(rotor->vehicle_mount_1.casted<ScenePos>()),
                    .p1 = T1.transform(rotor->blades_mount_1.casted<ScenePos>()),
                    .beta = c.cfg.point_equality_beta}));
        }
    }
    auto rbp_orig = rbp_;
    for (auto& [wing_id, wing] : wings_) {
        // Absolute location
        auto abs_location = wing->absolute_location(rbp_.abs_transformation());
        // Relative velocity
        auto vel = dot(rbp_orig.velocity_at_position(abs_location.t), abs_location.R);
        auto vel2 = squared(vel);
        auto lvel = std::sqrt(sum(squared(vel)));
        auto svel2 = lvel * vel;
        auto drag = -wing->drag_coefficients * svel2;
        float fac = wing->fac(lvel);
        auto thr = rbp_.mass_ * c.cfg.max_aerodynamic_acceleration;
        integrate_force(
            VectorAtPosition<float, ScenePos, 3>{
                .vector = abs_location.rotate(
                    clamped(
                        fac * FixedArray<float, 3>{
                            drag(0),
                            drag(1) - svel2(2) * wing->angle_of_attack * wing->angle_coefficient_yz + vel2(2) * wing->lift_coefficient,
                            drag(2) - svel2(2) * std::abs(wing->brake_angle) * wing->angle_coefficient_zz},
                        -thr,
                        thr)),
                .position = abs_location.t },
            c.cfg,
            c.phase);
        if (wing->trail_source.has_value()) {
            const auto& s = *wing->trail_source;
            if (std::abs(lvel) > s.minimum_velocity) {
                TransformationMatrix<float, ScenePos, 3> trail_location{
                    abs_location.R,
                    abs_location.transform(s.position.casted<ScenePos>()) };
                s.extender->append_location(trail_location, TrailLocationType::MIDPOINT, c.world);
            }
        }
    }
    if (!std::isnan(fly_forward_state_.wants_to_fly_forward_factor_)) {
        auto dir = rbp_.rotation_.column(1);
        if ((c.world.gravity == nullptr) || (c.world.gravity->magnitude == 0.f)) {
            THROW_OR_ABORT("collide_with_air without gravity");
        }
        dir -= dot0d(dir, c.world.gravity->direction) * c.world.gravity->direction;
        float l2 = sum(squared(dir));
        if (l2 > 1e-6) {
            integrate_force(
                VectorAtPosition<float, ScenePos, 3>{
                    .vector = - (fly_forward_state_.wants_to_fly_forward_factor_ /
                                std::sqrt(l2)) *
                                dir,
                    .position = abs_com() },
                c.cfg,
                c.phase);
        }
    }
    for (auto& [tire_id, tire] : tires_) {
        if (tire.rbp == nullptr) {
            continue;
        }
        auto T0 = rbp_.abs_transformation();
        auto T1 = tire.rbp->abs_transformation();
        auto abs_vehicle_mount_0 = T0.transform(tire.vehicle_mount_0.casted<ScenePos>());
        auto abs_vertical_line = T0.rotate(tire.vertical_line);
        auto rod0f = T0.rotate(tire.rotation_axis());
        auto rod0 = rod0f.casted<ScenePos>();
        auto rod1f = T1.R.column(0);
        auto rod1 = rod1f.casted<ScenePos>();
        if (WHEEL_CONSTRAINT_TYPE == WheelConstraintType::PLANES) {
            // Vertical constraints
            {
                c.contact_infos.push_back(std::make_unique<LineContactInfo2>(
                    rbp_,
                    *tire.rbp,
                    LineEqualityConstraint{
                        .pec = PointEqualityConstraint{
                            .p0 = abs_vehicle_mount_0,
                            .p1 = T1.t,
                            .beta = c.cfg.point_equality_beta
                        },
                        .null_space = abs_vertical_line
                    }));
                // Disabled because this code prevents all but the vertical axis
                // from rotating when the time step is small.
                // c.contact_infos.push_back(std::make_unique<LineContactInfo2>(
                //     rbp_,
                //     *tire.rbp,
                //     LineEqualityConstraint{
                //         .pec = PointEqualityConstraint{
                //             .p0 = T0.transform(tire.vehicle_mount_1.casted<ScenePos>()),
                //             .p1 = T1.t(),
                //             .beta = c.cfg.point_equality_beta
                //         },
                //         .null_space = abs_vertical_line
                //     }));
            }
            // Horizontal constraints
            {
                size_t npoints = 3;
                for (size_t point_id = 0; point_id < npoints; ++point_id) {
                    float angle = (float)point_id / (float)npoints * 2 * (float)M_PI;
                    FixedArray<float, 3> p1r{ 0.f, std::sin(angle), std::cos(angle) };
                    auto p1 = T1.transform((tire.radius * p1r).casted<ScenePos>());
                    auto p0 = p1 - rod0 * dot0d(rod0, p1 - abs_vehicle_mount_0);
                    c.contact_infos.push_back(std::make_unique<PlaneContactInfo2>(
                        rbp_,
                        *tire.rbp,
                        BoundedPlaneEqualityConstraint{
                            PlaneEqualityConstraint{
                                .pec = PointEqualityConstraint{
                                    .p0 = p0,
                                    .p1 = p1,
                                    .beta = c.cfg.plane_equality_beta
                                },
                                .plane_normal = rod0f
                            }
                        }));
                }
            }
        }
        if (WHEEL_CONSTRAINT_TYPE == WheelConstraintType::ROD) {
            // Line constraint
            {
                for (auto dr = -0.5; dr <= 0.5; dr += 1) {
                    auto p0 = abs_vehicle_mount_0 + rod0 * dr;
                    auto p1 = T1.t + rod1 * dr;
                    c.contact_infos.push_back(std::make_unique<LineContactInfo2>(
                        rbp_,
                        *tire.rbp,
                        LineEqualityConstraint{
                            .pec = PointEqualityConstraint{
                                .p0 = p0,
                                .p1 = p1,
                                .beta = c.cfg.point_equality_beta
                            },
                            .null_space = abs_vertical_line
                        }));
                }
            }
            // Horizontal constraints
            {
                for (auto dy = -0.5; dy <= 0.5; dy += 1) {
                    auto p0 = abs_vehicle_mount_0 + abs_vertical_line.casted<ScenePos>() * dy;
                    auto p1 = p0 - rod1 * dot0d(rod1, p0 - T1.t);
                    c.contact_infos.push_back(std::make_unique<PlaneContactInfo2>(
                        rbp_,
                        *tire.rbp,
                        BoundedPlaneEqualityConstraint{
                            PlaneEqualityConstraint{
                                .pec = PointEqualityConstraint{
                                    .p0 = p0,
                                    .p1 = p1,
                                    .beta = c.cfg.plane_equality_beta
                                },
                                .plane_normal = rod0f
                            }
                        }));
                }
            }
        }
        // Shock absorber constraint
        {
            auto ci = std::make_unique<ShockAbsorberContactInfo2>(
                rbp_,
                *tire.rbp,
                BoundedShockAbsorberConstraint{
                    .constraint{
                        .normal_impulse{.normal = -abs_vertical_line},
                        .fit = 1.f,
                        .distance = dot0d((abs_vehicle_mount_0 - tire.rbp->abs_position()).casted<float>(), abs_vertical_line),
                        .Ks = tire.sKs,
                        .Ka = tire.sKa,
                        .exponent = tire.sKe
                    },
                    .lambda_min = tire.rbp->mass_ * c.cfg.velocity_lambda_min,
                    .lambda_max = -tire.rbp->mass_ * c.cfg.velocity_lambda_min },
                tire.rbp->abs_position());
            tire.normal_impulse = &ci->normal_impulse();
            c.contact_infos.push_back(std::move(ci));
        }
    }
}

void RigidBodyVehicle::advance_time_skate(
    const PhysicsEngineConfig& cfg,
    const StaticWorld& world)
{
    // Revert surface power
    if ((revert_surface_power_state_.revert_surface_power_threshold_ != INFINITY) &&
        (!grind_state_.grinding_ || (grind_state_.grind_axis_ == 2)))
    {
        float f = dot0d(rbp_.v_com_, dot1d(rbp_.rotation_, tires_z_));
        if (!revert_surface_power_state_.revert_surface_power_) {
            f = -f;
        }
        if (f > revert_surface_power_state_.revert_surface_power_threshold_) {
            revert_surface_power_state_.revert_surface_power_ = !revert_surface_power_state_.revert_surface_power_;
        }
    }
    // Align to surface
    if (grind_state_.grinding_) {
        if ((world.gravity == nullptr) || (world.gravity->magnitude == 0.f)) {
            THROW_OR_ABORT("advance_time_skate without gravity");
        }
        if (grind_state_.grind_axis_ == 0) {
            if (std::abs(grind_state_.grind_pv_(0)) > 1e-12) {
                auto x = cross(sign(grind_state_.grind_pv_(0)) * grind_state_.grind_direction_, world.gravity->direction);
                x /= std::sqrt(sum(squared(x)));
                auto z = cross(x, world.gravity->direction);
                auto r1 = FixedArray<float, 3, 3>::init(
                    -z(0), -world.gravity->direction(0), -x(0),
                    -z(1), -world.gravity->direction(1), -x(1),
                    -z(2), -world.gravity->direction(2), -x(2));
                rbp_.rotation_ =
                    Quaternion<float>{ rbp_.rotation_ }
                    .slerp(Quaternion<float>{ r1 }, cfg.alignment_slerp)
                    .to_rotation_matrix();
            }
        } else if (grind_state_.grind_axis_ == 2) {
            if (std::abs(grind_state_.grind_pv_(2)) > 1e-12) {
                auto r1 = gl_lookat_relative(-sign(grind_state_.grind_pv_(2)) * grind_state_.grind_direction_, -world.gravity->direction);
                if (!r1.has_value()) {
                    THROW_OR_ABORT("Could not compute grind rotation");
                }
                rbp_.rotation_ =
                    Quaternion<float>{ rbp_.rotation_ }
                    .slerp(Quaternion<float>{ *r1 }, cfg.alignment_slerp)
                    .to_rotation_matrix();
            }
        } else {
            THROW_OR_ABORT("Unknown grind axis");
        }
    } else {
        if ((align_to_surface_state_.align_to_surface_relaxation_ != 0) &&
            !all(Mlib::isnan(align_to_surface_state_.surface_normal_)))
        {
            if (!all(rbp_.w_ == 0.f)) {
                THROW_OR_ABORT("Detected angular velocity despite alignment to surface normal. Forgot to set the rigid body's size to INFINITY?");
            }
            rbp_.rotation_ = rotate_axis_onto_other_axis(
                rbp_.rotation_,
                align_to_surface_state_.surface_normal_,
                FixedArray<float, 3>{ 0.f, 1.f, 0.f },
                align_to_surface_state_.align_to_surface_relaxation_);
        }
    }
}

void RigidBodyVehicle::finalize_collisions(CollisionHistory& c) {
    if (!c.phase.group.rigid_bodies.contains(&rbp_)) {
        return;
    }
    if (is_deactivated_avatar()) {
        THROW_OR_ABORT("Attempt to finalize collisions of deactivated avatar");
    }
    for (auto& [tire_id, tire] : tires_) {
        if (tire.rbp != nullptr) {
            update_tire_angular_velocity(tire_id);
        }
    }
}

void RigidBodyVehicle::advance_time(
    const PhysicsEngineConfig& cfg,
    const StaticWorld& world,
    std::list<Beacon>* beacons,
    const PhysicsPhase& phase)
{
    if (!phase.group.rigid_bodies.contains(&rbp_)) {
        return;
    }
    if (is_deactivated_avatar()) {
        THROW_OR_ABORT("Attempt to move deactivated avatar");
    }
    auto time_step = PhysicsTimeStep{
        .dt_step = cfg.dt,
        .dt_substep = cfg.dt_substeps(phase)
    };
    auto dt_substeps = cfg.dt_substeps(phase);
    advance_time_skate(cfg, world);
    rbp_.advance_time(dt_substeps);
    for (auto& [_, t] : tires_) {
        t.advance_time(dt_substeps);
        if (t.rbp != nullptr) {
            auto rel_tire_pos = rbp_.abs_transformation().itransform(t.rbp->abs_position()).casted<float>();
            t.shock_absorber_position = dot0d(rel_tire_pos - t.vehicle_mount_0, t.vertical_line);
        }
    }
    {
        auto frame = rbp_.rotating_frame();
        for (auto& [_, e] : engines_) {
            e.advance_time(time_step, phase, frame, world);
        }
    }
#ifdef COMPUTE_POWER
    {
        float nrg = energy();
        if (!std::isnan(energy_old_)) {
            power_ = (nrg - energy_old_) / dt;
        }
        energy_old_ = nrg;
    }
#endif
}

float RigidBodyVehicle::mass() const {
    return rbp_.mass_;
}

FixedArray<ScenePos, 3> RigidBodyVehicle::abs_com() const {
    return rbp_.abs_com_;
}

FixedArray<float, 3, 3> RigidBodyVehicle::abs_I() const {
    return rbp_.abs_I();
}

FixedArray<ScenePos, 3> RigidBodyVehicle::abs_grind_point() const {
    if (!grind_state_.grind_point_.has_value()) {
        THROW_OR_ABORT("Grind point is not set");
    }
    return rbp_.transform_to_world_coordinates(*grind_state_.grind_point_);
}

FixedArray<ScenePos, 3> RigidBodyVehicle::abs_target() const {
    return rbp_.transform_to_world_coordinates(target_);
}

VectorAtPosition<float, ScenePos, 3> RigidBodyVehicle::abs_F(const VectorAtPosition<float, ScenePos, 3>& F) const {
    return {
        .vector = dot1d(rbp_.rotation_, F.vector),
        .position = dot1d(rbp_.rotation_.casted<ScenePos>(), F.position) + rbp_.abs_position()};
}

FixedArray<float, 3> RigidBodyVehicle::velocity_at_position(const FixedArray<ScenePos, 3>& position) const {
    return rbp_.velocity_at_position(position);
}

void RigidBodyVehicle::set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) {
    rbp_.set_pose(
        absolute_model_matrix.R,
        absolute_model_matrix.t);
}

TransformationMatrix<float, ScenePos, 3> RigidBodyVehicle::get_new_absolute_model_matrix() const {
    return rbp_.abs_transformation();
}

void RigidBodyVehicle::set_max_velocity(float max_velocity) {
    max_velocity_ = max_velocity;
}

void RigidBodyVehicle::set_tire_angle_y(size_t id, float angle_y) {
    get_tire(id).angle_y = angle_y;
}

// void RigidBodyVehicle::set_tire_accel_x(size_t id, float accel_x) {
//     get_tire(id).accel_x = accel_x;
// }

void RigidBodyVehicle::set_rotor_angle_x(size_t id, float angle_x) {
    get_rotor(id).angles(0) = angle_x;
}

void RigidBodyVehicle::set_rotor_angle_y(size_t id, float angle_y) {
    get_rotor(id).angles(1) = angle_y;
}

void RigidBodyVehicle::set_rotor_angle_z(size_t id, float angle_z) {
    get_rotor(id).angles(2) = angle_z;
}

void RigidBodyVehicle::set_rotor_movement_x(size_t id, float movement_x) {
    get_rotor(id).movement(0) = movement_x;
}

void RigidBodyVehicle::set_rotor_movement_y(size_t id, float movement_y) {
    get_rotor(id).movement(1) = movement_y;
}

void RigidBodyVehicle::set_rotor_movement_z(size_t id, float movement_z) {
    get_rotor(id).movement(2) = movement_z;
}

void RigidBodyVehicle::set_wing_angle_of_attack(size_t id, float angle) {
    if (std::abs(angle) >= M_PI) {
        THROW_OR_ABORT("Angle of attack too large");
    }
    get_wing(id).angle_of_attack = angle;
}

void RigidBodyVehicle::set_wing_brake_angle(size_t id, float angle) {
    if (std::abs(angle) >= M_PI) {
        THROW_OR_ABORT("Brake angle too large");
    }
    get_wing(id).brake_angle = angle;
}

FixedArray<float, 3> RigidBodyVehicle::get_abs_tire_z(size_t id) const {
    FixedArray<float, 3> z{ tires_z_ };
    const auto& t = tires_.get(id);
    z = dot1d(rodrigues2(t.vertical_line, t.angle_y), z);
    return dot1d(rbp_.rotation_, z);
}

float RigidBodyVehicle::get_tire_angular_velocity(size_t id) const {
    verify_tire_angular_velocity(id);
    return get_tire(id).angular_velocity;
}

void RigidBodyVehicle::update_tire_angular_velocity(size_t id) {
    auto& tire = get_tire(id);
    if (tire.rbp == nullptr) {
        std::stringstream sstr;
        sstr <<
            "Vehicle \"" << name() <<
            "\": tire " << id <<
            " has no rigid body";
        THROW_OR_ABORT(sstr.str());
    }
    auto abs_rotation_axis = dot1d(rbp_.rotation_, tire.rotation_axis());
    tire.angular_velocity = dot0d(tire.rbp->w_, abs_rotation_axis);
}

void RigidBodyVehicle::verify_tire_angular_velocity(size_t id) const {
    const auto& tire = get_tire(id);
    if (tire.rbp != nullptr) {
        auto abs_rotation_axis = dot1d(rbp_.rotation_, tire.rotation_axis());
        if (std::abs((tire.angular_velocity / rpm) - (dot0d(tire.rbp->w_, abs_rotation_axis) / rpm)) > 0.1) {
            std::stringstream sstr;
            sstr <<
                "Last update: " << tire.rbp->last_update_source_location_ <<
                " vehicle: \"" << name() <<
                "\": tire " << id <<
                " scalar RPM: " << (tire.angular_velocity / rpm) <<
                " vectorial RPM: " << (dot0d(tire.rbp->w_, abs_rotation_axis) / rpm);
            THROW_OR_ABORT(sstr.str());
        }
    }
}

void RigidBodyVehicle::set_tire_angular_velocity(
    size_t id,
    float w,
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase,
    float& available_power)
{
    auto& tire = get_tire(id);
    set_base_angular_velocity(tire, tire.rotation_axis(), w, cfg, phase, available_power);
}

void RigidBodyVehicle::set_rotor_angular_velocity(
    size_t id,
    float w,
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase,
    float& available_power)
{
    auto& rotor = get_rotor(id);
    set_base_angular_velocity(rotor, rotor.rotation_axis(), w, cfg, phase, available_power);
}

void RigidBodyVehicle::set_base_angular_velocity(
    BaseRotor& base_rotor,
    const FixedArray<float, 3>& rotation_axis,
    float w,
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase,
    float& available_power)
{
    if (base_rotor.rbp != nullptr) {
        auto abs_rotation_axis = dot1d(rbp_.rotation_, rotation_axis);
        if (false) {
            auto I = dot0d(abs_rotation_axis, base_rotor.rbp->dot1d_abs_I(abs_rotation_axis));
            float dw = w - base_rotor.angular_velocity;
            if (sign(dw) != sign(available_power)) {
                THROW_OR_ABORT("Sign mismatch between dw and available power");
            }
            float available_torque;
            bool requires_power = (sign(dw) == sign(base_rotor.angular_velocity));
            if (requires_power) {
                // tau = r * F = dL / dt
                // dE = F * r * dQ          // dQ: Angle
                // P * dt = dL / dt * dQ
                //        = dL * dQ / dt
                //        = dL * w
                // P = dL / dt * w
                //   = tau * w
                available_torque = std::abs(available_power) / std::max((float)1e-6, std::abs(base_rotor.angular_velocity));
            } else {
                available_torque = base_rotor.brake_torque;
            }
            auto available_dL = std::copysign(
                std::min(
                    std::abs(dw) / I,
                    available_torque * cfg.dt_substeps(phase)),
                dw);
            if (requires_power) {
                auto ap = std::abs(available_power) - available_dL / cfg.dt_substeps(phase) * base_rotor.angular_velocity;
                if (ap < 1e-12) {
                    THROW_OR_ABORT("available_power too strongly negative");
                }
                available_power = std::copysign(ap, available_power);
            }
            auto dL_vec = available_dL * abs_rotation_axis;
            auto dt = cfg.dt_substeps(phase);
            base_rotor.rbp->integrate_delta_angular_momentum(dL_vec, 0.f, dt);
            rbp_.integrate_delta_angular_momentum(dL_vec, 0.f, dt);
            base_rotor.angular_velocity = dot0d(base_rotor.rbp->w_, abs_rotation_axis);
        } else {
            // Change angular velocity along the rotation axis, while preserving the remaining components.
            base_rotor.rbp->w_ += abs_rotation_axis * (w - dot0d(base_rotor.rbp->w_, abs_rotation_axis));
            base_rotor.angular_velocity = w;
        }
    } else {
        base_rotor.angular_velocity = w;
    }
}

FixedArray<float, 3> RigidBodyVehicle::get_velocity_at_tire_contact(
    const FixedArray<float, 3>& surface_normal,
    size_t id) const
{
    FixedArray<float, 3> v = velocity_at_position(get_abs_tire_contact_position(id));
    v -= surface_normal * dot0d(v, surface_normal);
    return v;
}

float RigidBodyVehicle::get_angular_velocity_at_tire(
    const FixedArray<float, 3>& surface_normal,
    const FixedArray<float, 3>& street_velocity,
    size_t id) const
{
    auto z = get_abs_tire_z(id);
    auto v = get_velocity_at_tire_contact(surface_normal, id) - street_velocity;
    return -dot0d(v, z) / get_tire_radius(id);
}

float RigidBodyVehicle::get_tire_radius(size_t id) const {
    return get_tire(id).radius;
}

TirePowerIntent RigidBodyVehicle::consume_tire_surface_power(
    size_t id,
    VelocityClassification velocity_classification)
{
    Tire& tire = get_tire(id);
    auto& e = engines_.get(tire.engine);
    const RigidBodyDeltaEngine* de = nullptr;
    if (tire.delta_engine.has_value()) {
        de = &delta_engines_.get(*tire.delta_engine);
    }
    return e.consume_tire_power(
        id,
        &tire.angular_velocity,
        de == nullptr
            ? EnginePowerDeltaIntent::zero()
            : de->engine_power_delta_intent(),
        velocity_classification);
}

TirePowerIntent RigidBodyVehicle::consume_rotor_surface_power(size_t id) {
    Rotor& rotor = get_rotor(id);
    auto& e = engines_.get(rotor.engine);
    const RigidBodyDeltaEngine* de = nullptr;
    if (rotor.delta_engine.has_value()) {
        de = &delta_engines_.get(*rotor.delta_engine);
    }
    return e.consume_rotor_power(
        id,
        &rotor.angular_velocity,
        de == nullptr
            ? EnginePowerDeltaIntent::zero()
            : de->engine_power_delta_intent());
}

void RigidBodyVehicle::set_surface_power(
    const VariableAndHash<std::string>& engine_name,
    const EnginePowerIntent& engine_power_intent)
{
    auto& e = engines_.get(engine_name);
    e.set_surface_power(
        EnginePowerIntent{
            .state = engine_power_intent.state,
            .surface_power = revert_surface_power_state_.revert_surface_power_
                ? -engine_power_intent.surface_power
                : engine_power_intent.surface_power,
            .drive_relaxation = engine_power_intent.drive_relaxation,
            .parking_brake_pulled = engine_power_intent.parking_brake_pulled});
}

void RigidBodyVehicle::set_delta_surface_power(
    const VariableAndHash<std::string>& delta_engine_name,
    const EnginePowerDeltaIntent& engine_power_delta_intent)
{
    auto& e = delta_engines_.get(delta_engine_name);
    e.set_surface_power(
        EnginePowerDeltaIntent{
            .delta_power = engine_power_delta_intent.delta_power,
            .delta_relaxation = engine_power_delta_intent.delta_relaxation});
}

void RigidBodyVehicle::park_vehicle() {
    for (const auto& [n, _] : engines_) {
        set_surface_power(n, EnginePowerIntent{
            .state = EngineState::OFF,
            .surface_power = 0.f,
            .drive_relaxation = 0.f,
            .parking_brake_pulled = 1.f});
    }
}

float RigidBodyVehicle::get_tire_break_force(size_t id) const {
    return get_tire(id).brake_force;
}

FixedArray<ScenePos, 3> RigidBodyVehicle::get_abs_tire_contact_position(size_t id) const {
    const Tire& tire = get_tire(id);
    return rbp_.transform_to_world_coordinates(
        tire.vehicle_mount_0 +
        tire.vertical_line * (tire.shock_absorber_position - tire.radius));
}

const Tire& RigidBodyVehicle::get_tire(size_t id) const {
    return const_cast<RigidBodyVehicle*>(this)->get_tire(id);
}

Tire& RigidBodyVehicle::get_tire(size_t id) {
    return tires_.get(id);
}

const Rotor& RigidBodyVehicle::get_rotor(size_t id) const {
    return const_cast<RigidBodyVehicle*>(this)->get_rotor(id);
}

Rotor& RigidBodyVehicle::get_rotor(size_t id) {
    return *rotors_.get(id);
}

const Wing& RigidBodyVehicle::get_wing(size_t id) const {
    return const_cast<RigidBodyVehicle*>(this)->get_wing(id);
}

Wing& RigidBodyVehicle::get_wing(size_t id) {
    return *wings_.get(id);
}

float RigidBodyVehicle::energy() const {
    return rbp_.energy();
}

const std::string& RigidBodyVehicle::name() const {
    return name_;
}

const std::string& RigidBodyVehicle::asset_id() const {
    return asset_id_;
}

// void RigidBodyVehicle::set_tire_sliding(size_t id, bool value) {
//     tire_sliding_[id] = value;
// }
// bool RigidBodyVehicle::get_tire_sliding(size_t id) const {
//     auto t = tire_sliding_.find(id);
//     if (t != tire_sliding_.end()) {
//         return t->second;
//     }
//     return false;
// }

void RigidBodyVehicle::write_status(
    std::ostream& ostr,
    StatusComponents log_components,
    const StaticWorld& world) const
{
    if (log_components & StatusComponents::TIME) {
        static const std::chrono::steady_clock::time_point epoch_time = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - epoch_time);
        ostr << "t: " << milliseconds.count() << " ms" << std::endl;
    }
    // if (true) {
    //     static std::chrono::time_point time_v0 = std::chrono::steady_clock::time_point();
    //     static std::chrono::time_point time_v100 = std::chrono::steady_clock::time_point();
    //     float v = std::sqrt(sum(squared(rbp_.v_)));
    //     if (v < 1.f * kph) {
    //         time_v0 = std::chrono::steady_clock::now();
    //     }
    //     if (std::abs(v - 100.f * kph) < 1.f * kph) {
    //         time_v100 = std::chrono::steady_clock::now();
    //     }
    //     if ((time_v0 != std::chrono::steady_clock::time_point()) &&
    //         (time_v100 != std::chrono::steady_clock::time_point()))
    //     {
    //         if (time_v0 < time_v100) {
    //             ostr << "t 0-100: " << (time_v100 - time_v0).count() << " s" << std::endl;
    //         } else if (time_v100 < time_v0) {
    //             ostr << "t 100-0: " << (time_v0 - time_v100).count() << " s" << std::endl;
    //         }
    //     }
    // }
    // if (true) {
    //     float dt = 1.f / 60.f * s;
    //     static FixedArray<float, 3> old_velocity;
    //     auto a = std::sqrt(sum(squared((rbp_.v_ - old_velocity)))) / dt;
    //     old_velocity = rbp_.v_;
    //     ostr << "a: " << a / (meters / (s * s)) << " m/s^2" << std::endl;
    // }
    if (log_components & StatusComponents::SPEED) {
        ostr << "v: " << std::sqrt(sum(squared(rbp_.v_com_))) / kph << " km/h" << std::endl;
    }
    if (log_components & StatusComponents::ANGULAR_VELOCITY) {
        ostr << "w: " << std::sqrt(sum(squared(rbp_.w_))) / rpm << " rpm" << std::endl;
    }
    if (log_components & StatusComponents::WHEEL_ANGULAR_VELOCITY) {
        ostr << "wt: " << std::sqrt(sum(squared(rbp_.v_com_))) / WHEEL_RADIUS / rpm << " rpm" << std::endl;
    }
    if (log_components & StatusComponents::DIAMETER) {
        if ((world.gravity == nullptr) || (world.gravity->magnitude == 0.f)) {
            THROW_OR_ABORT("StatusComponents::DIAMETER without gravity");
        }
        // T = 2 PI r / v, T = 2 PI / w
        // r = v / w
        // r / r2 = v * a / (w * v^2) = a / (w * v)
        if (float w2 = sum(squared(rbp_.w_)); w2 > squared(0.001f * rpm)) {
            ostr << "d: " << 2 * std::sqrt(sum(squared(rbp_.v_com_)) / w2) << " m" << std::endl;
            ostr << "d / d2(g): " << world.gravity->magnitude / std::sqrt(w2 * sum(squared(rbp_.v_com_))) << std::endl;
        } else {
            ostr << "d: undefined" << std::endl;
            ostr << "d / d2(g): undefined" << std::endl;
        }
    }
    // if (log_components & StatusComponents::DIAMETER2) {
    //     // F = m * a = m v^2 / r
    //     // r = v^2 / a
    //     if (float a2 = sum(squared(rbi_.a_)); a2 > squared(0.01f * meters / (s * s))) {
    //         ostr << "d2: " << 2 * sum(squared(rbp_.v_)) / std::sqrt(a2) / meters << " m" << std::endl;
    //     } else {
    //         ostr << "d2: undefined" << std::endl;
    //     }
    //     // Not implemented: https://de.wikipedia.org/wiki/Wendekreis_(Fahrzeug)
    //     // D = 2 L / sin(alpha)
    // }
    if (log_components & StatusComponents::POSITION) {
        auto pos = rbp_.abs_position();
        ostr << "x: " << pos(0) / meters << " m" << std::endl;
        ostr << "y: " << pos(1) / meters << " m" << std::endl;
        ostr << "z: " << pos(2) / meters << " m" << std::endl;
        if (geographic_mapping_ != nullptr) {
            auto gpos = geographic_mapping_->transform(pos.casted<double>());
            ostr << "lat: " << latitude_to_string(gpos(0)) << std::endl;
            ostr << "lon: " << longitude_to_string(gpos(1)) << std::endl;
            ostr << "height: " << height_to_string(gpos(2)) << std::endl;
        }
    }
    if (log_components & StatusComponents::ENERGY) {
        ostr << "E: " << energy() / (kilo * J) << " kJ" << std::endl;
#ifdef COMPUTE_POWER
        // ostr << "P: " << power_ / W << " W" << std::endl;
        if (!std::isnan(power_)) {
            ostr << "P: " << power_ / hp << " hp" << std::endl;
        }
#endif
    }
    if (log_components & StatusComponents::DRIVER_NAME) {
        if (auto driver = drivers_.try_get("driver"); driver != nullptr) {
            ostr << "Driver: " << driver->title() << std::endl;
        }
    }
    for (const auto& o : collision_observers_) {
        auto c = dynamic_cast<StatusWriter*>(o.get());
        if (c != nullptr) {
            c->write_status(ostr, log_components, world);
        }
    }
    {
        auto c = dynamic_cast<StatusWriter*>(damageable_);
        if (c != nullptr) {
            c->write_status(ostr, log_components, world);
        }
    }
}

float RigidBodyVehicle::get_value(StatusComponents status_components) const {
    if (status_components == StatusComponents::SPEED) {
        return std::sqrt(sum(squared(rbp_.v_com_))) / kph;
    }
    THROW_OR_ABORT("Unsupported status component: " + std::to_string((unsigned int)status_components));
}

static auto engines_name = VariableAndHash<std::string>{ "engines" };

StatusWriter& RigidBodyVehicle::child_status_writer(const std::vector<VariableAndHash<std::string>>& name) {
    if (name.size() != 2) {
        THROW_OR_ABORT("Unknown child status writer");
    }
    if (name[0] != engines_name) {
        THROW_OR_ABORT("Unknown child status writer");
    }
    return engines_.get(name[1]);
}

bool RigidBodyVehicle::node_shall_be_hidden(
    const DanglingBaseClassPtr<const SceneNode>& camera_node,
    const ExternalRenderPass& external_render_pass) const
{
    std::scoped_lock lock{ flags_mutex_ };
    return is_deactivated_avatar();
}

bool RigidBodyVehicle::feels_gravity() const {
    return !any(flags_ & RigidBodyVehicleFlags::FEELS_NO_GRAVITY);
}

bool RigidBodyVehicle::is_avatar() const {
    return any(flags_ & RigidBodyVehicleFlags::IS_ANY_AVATAR);
}

bool RigidBodyVehicle::is_activated_avatar() const {
    if (!is_avatar()) {
        return false;
    }
    return any(flags_ & RigidBodyVehicleFlags::IS_ACTIVATED_AVATAR);
}

bool RigidBodyVehicle::is_deactivated_avatar() const {
    if (!is_avatar()) {
        return false;
    }
    return any(flags_ & RigidBodyVehicleFlags::IS_DEACTIVATED_AVATAR);
}

bool RigidBodyVehicle::has_avatar_controller() const {
    return avatar_controller_ != nullptr;
}

bool RigidBodyVehicle::has_vehicle_controller() const {
    return vehicle_controller_ != nullptr;
}

bool RigidBodyVehicle::has_plane_controller() const {
    return plane_controller_ != nullptr;
}

bool RigidBodyVehicle::has_missile_controller() const {
    return missile_controller_ != nullptr;
}

RigidBodyAvatarController& RigidBodyVehicle::avatar_controller() {
    if (avatar_controller_ == nullptr) {
        THROW_OR_ABORT("Rigid body \"" + name() + "\" has no avatar controller");
    }
    return *avatar_controller_;
}

RigidBodyPlaneController& RigidBodyVehicle::plane_controller() {
    if (plane_controller_ == nullptr) {
        THROW_OR_ABORT("Rigid body \"" + name() + "\" has no plane controller");
    }
    return *plane_controller_;
}

RigidBodyVehicleController& RigidBodyVehicle::vehicle_controller() {
    if (vehicle_controller_ == nullptr) {
        THROW_OR_ABORT("Rigid body \"" + name() + "\" has no vehicle controller");
    }
    return *vehicle_controller_;
}

RigidBodyMissileController& RigidBodyVehicle::missile_controller() {
    if (missile_controller_ == nullptr) {
        THROW_OR_ABORT("Rigid body \"" + name() + "\" has no missile controller");
    }
    return *missile_controller_;
}

void RigidBodyVehicle::deactivate_avatar() {
    std::scoped_lock lock{ flags_mutex_ };
    if (!any(flags_ & RigidBodyVehicleFlags::IS_ACTIVATED_AVATAR)) {
        THROW_OR_ABORT("Rigid body vehicle is not an activated avatar");
    }
    flags_ &= ~RigidBodyVehicleFlags::IS_ACTIVATED_AVATAR;
    flags_ |= RigidBodyVehicleFlags::IS_DEACTIVATED_AVATAR;
}

void RigidBodyVehicle::activate_avatar() {
    std::scoped_lock lock{ flags_mutex_ };
    if (!any(flags_ & RigidBodyVehicleFlags::IS_DEACTIVATED_AVATAR)) {
        THROW_OR_ABORT("Rigid body vehicle is not a deactivated avatar");
    }
    flags_ &= ~RigidBodyVehicleFlags::IS_DEACTIVATED_AVATAR;
    flags_ |= RigidBodyVehicleFlags::IS_ACTIVATED_AVATAR;
}

void RigidBodyVehicle::add_autopilot(const DanglingBaseClassRef<IVehicleAi>& ai)
{
    for (const auto& sf : ai->skills()) {
        auto& amap = autopilots_[sf.scenario.actor_task];
        auto it0 = amap.find(sf.scenario.actor_type);
        if ((it0 == amap.end()) || (sf.factor > it0->second.skill)) {
            auto it1 = amap.try_emplace(
                sf.scenario.actor_type,
                ai,
                CURRENT_SOURCE_LOCATION,
                sf.factor);
            if (!it1.second) {
                verbose_abort("Fatal error, could not insert autopilot");
            }
            it1.first->second.ai.on_destroy([this, s=sf.scenario]() { remove_autopilot(s); }, CURRENT_SOURCE_LOCATION);
        }
    }
}

DanglingBaseClassRef<IVehicleAi> RigidBodyVehicle::get_autopilot(const SkillScenario& scenario)
{
    auto dit = autopilots_.find(scenario.actor_task);
    if (dit == autopilots_.end()) {
        THROW_OR_ABORT("No autopilot for scenario \"" + skill_scenario_to_string(scenario) + "\" exists");
    }
    auto vit = dit->second.find(scenario.actor_type);
    if (vit == dit->second.end()) {
        THROW_OR_ABORT("No autopilot for scenario \"" + skill_scenario_to_string(scenario) + "\" exists");
    }
    return vit->second.ai.object();
}

bool RigidBodyVehicle::has_autopilot(const ActorTask& actor_task) const {
    return autopilots_.contains(actor_task);
}

bool RigidBodyVehicle::has_autopilot(const SkillScenario& scenario) const {
    auto dit = autopilots_.find(scenario.actor_task);
    if (dit == autopilots_.end()) {
        return false;
    }
    auto vit = dit->second.find(scenario.actor_type);
    if (vit == dit->second.end()) {
        return false;
    }
    return true;
}

void RigidBodyVehicle::remove_autopilot(const SkillScenario& scenario) {
    auto dit = autopilots_.find(scenario.actor_task);
    if (dit == autopilots_.end()) {
        verbose_abort("Could not remove autopilot with scenario \"" + skill_scenario_to_string(scenario) + '"');
    }
    auto vit = dit->second.find(scenario.actor_type);
    if (vit == dit->second.end()) {
        verbose_abort("Could not remove autopilot with scenario \"" + skill_scenario_to_string(scenario) + '"');
    }
    dit->second.erase(vit);
    if (dit->second.empty()) {
        autopilots_.erase(dit);
    }
}

VehicleAiMoveToStatus RigidBodyVehicle::move_to(
    const AiWaypoint& ai_waypoint,
    const SkillMap* skills,
    const StaticWorld& world,
    float dt)
{
    auto it = autopilots_.find(actor_task_);
    if (it == autopilots_.end()) {
        return VehicleAiMoveToStatus::AUTOPILOT_IS_NULL;
    }
    auto status = VehicleAiMoveToStatus::NONE;
    for (auto& ai : it->second) {
        status |= ai.second.ai->move_to(ai_waypoint, skills, world, dt);
    }
    return status;
}

void RigidBodyVehicle::set_actor_task(ActorTask actor_task) {
    actor_task_ = actor_task;
}

void RigidBodyVehicle::set_waypoint_ofs(CompressedScenePos dy) {
    waypoint_ofs_ = dy;
}

void RigidBodyVehicle::set_surface_normal(std::unique_ptr<ISurfaceNormal>&& surface_normal) {
    if (surface_normal_ != nullptr) {
        THROW_OR_ABORT("Surface normal already set");
    }
    surface_normal_ = std::move(surface_normal);
}

bool RigidBodyVehicle::has_surface_normal() const {
    return surface_normal_ != nullptr;
}

const ISurfaceNormal& RigidBodyVehicle::get_surface_normal() const {
    if (surface_normal_ == nullptr) {
        THROW_OR_ABORT("Rigid body has no surface normal");
    }
    return *surface_normal_;
}

void RigidBodyVehicle::set_collision_normal_modifier(std::unique_ptr<ICollisionNormalModifier>&& collision_normal_modifier) {
    if (collision_normal_modifier_ != nullptr) {
        THROW_OR_ABORT("Collision normal modifier already set");
    }
    collision_normal_modifier_ = std::move(collision_normal_modifier);
}

bool RigidBodyVehicle::has_collision_normal_modifier() const {
    return collision_normal_modifier_ != nullptr;
}

const ICollisionNormalModifier& RigidBodyVehicle::get_collision_normal_modifier() const {
    if (collision_normal_modifier_ == nullptr) {
        THROW_OR_ABORT("Rigid body has no collision normal modifier");
    }
    return *collision_normal_modifier_;
}

void RigidBodyVehicle::get_rigid_pulses(std::unordered_set<RigidBodyPulses*>& rbps) {
    rbps.insert(&rbp_);
    for (auto& [_, t] : tires_) {
        if (t.rbp != nullptr) {
            rbps.insert(t.rbp);
        }
    }
    for (auto& [_, r] : rotors_) {
        if (r->rbp != nullptr) {
            rbps.insert(r->rbp);
        }
    }
}

FixedArray<float, 3> TrailerHitches::get_position_female() const {
    if (!female_.has_value()) {
        THROW_OR_ABORT("Vehicle has no female trailer hitch");
    }
    return *female_;
}

FixedArray<float, 3> TrailerHitches::get_position_male() const {
    if (!male_.has_value()) {
        THROW_OR_ABORT("Vehicle has no male trailer hitch");
    }
    return *male_;
}

void TrailerHitches::set_position_female(const FixedArray<float, 3>& position) {
    if (female_.has_value()) {
        THROW_OR_ABORT("Female trailer hitch position already set");
    }
    female_ = position;
}

void TrailerHitches::set_position_male(const FixedArray<float, 3>& position) {
    if (male_.has_value()) {
        THROW_OR_ABORT("Male trailer hitch position already set");
    }
    male_ = position;
}

void RigidBodyVehicle::set_scene_node(
    Scene& scene,
    const DanglingBaseClassRef<SceneNode>& node,
    VariableAndHash<std::string> node_name,
    SourceLocation loc)
{
    if (scene_ != nullptr) {
        THROW_OR_ABORT("RigidBodyVehicle::set_scene_node: Scene already set");
    }
    scene_ = &scene;
    scene_node_ = node.ptr();
    node_name_ = std::move(node_name);

    node->set_absolute_movable({ *this, loc });
    if (is_avatar()) {
        node->insert_node_hider(nullptr, { *this, loc });
    }
    on_clear_scene_node_.set(node->on_clear, loc);
    on_clear_scene_node_.add([this, node](){
        if (node->has_absolute_movable()) {
            if (&node->get_absolute_movable() != this) {
                verbose_abort("Unexpected absolute movable");
            }
            node->clear_absolute_movable();
        }
        if (node->contains_node_hider(nullptr, { *this, CURRENT_SOURCE_LOCATION })) {
            node->remove_node_hider(nullptr, { *this, CURRENT_SOURCE_LOCATION });
        }
        if (scene_ == nullptr) {
            verbose_abort("RigidBodyVehicle::set_scene_node: Scene is null");
        }
        scene_ = nullptr;
        scene_node_ = nullptr;
        animation_state_updater_ = nullptr;
        object_pool_.remove(this);
    }, loc);
}
