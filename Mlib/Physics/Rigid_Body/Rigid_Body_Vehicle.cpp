#include "Rigid_Body_Vehicle.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Geographic_Coordinates.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Physics/Actuators/Base_Rotor.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Actuators/Rotor.hpp>
#include <Mlib/Physics/Actuators/Wing.hpp>
#include <Mlib/Physics/Collision/Constraints.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Type.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Avatar_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Plane_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <chrono>

static const float WHEEL_RADIUS = 0.25f;

using namespace Mlib;

RigidBodyVehicle::RigidBodyVehicle(
    const RigidBodyIntegrator& rbi,
    const std::string& name,
    const TransformationMatrix<double, double, 3>* geographic_mapping)
: rigid_bodies_{ nullptr },
  max_velocity_{ INFINITY },
#ifdef COMPUTE_POWER
  power_{ NAN },
  energy_old_{ NAN },
#endif
  tires_z_{ 0.f, 0.f, -1.f },
  target_{ 0.f, 0.f, 0.f },
  rbi_{ rbi },
  name_{ name },
  damageable_{ nullptr },
  animation_state_updater_{ nullptr },
  driver_{ nullptr },
  avatar_controller_{ nullptr},
  vehicle_controller_{ nullptr},
  jump_state_{
      .wants_to_jump_{ false },
      .wants_to_jump_oversampled_{ false },
      .jumping_counter_{ SIZE_MAX },
  },
  grind_state_ {
      .wants_to_grind_{ false },
      .wants_to_grind_counter_{ 0 },
      .grinding_{ false },
  },
  align_to_surface_state_{
      .align_to_surface_relaxation_{ 0.f },
      .touches_alignment_plane_{ false },
      .surface_normal_{ NAN, NAN, NAN },
  },
  revert_surface_power_state_{
      .revert_surface_power_threshold_{ INFINITY },
      .revert_surface_power_{ false },
  },
  fly_forward_state_{
      .wants_to_fly_forward_factor_ = NAN
  },
  geographic_mapping_{ geographic_mapping }
{
    if (name.empty()) {
        throw std::runtime_error("No name given for rigid body vehicle");
    }
    // std::cerr << "Rigid body vehicle \"" << name << '"' << std::endl;
    // std::cerr << "I [kg*m??]\n" << rbi.rbp_.I_ / (kg * squared(meters)) << std::endl;
}

RigidBodyVehicle::~RigidBodyVehicle()
{
    if (driver_ != nullptr) {
        driver_->notify_vehicle_destroyed();
    }
}

void RigidBodyVehicle::reset_forces(size_t oversampling_iteration) {
    rbi_.reset_forces();
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
        ++jump_state_.jumping_counter_;
    }

    // Must be below the block above.
    if (oversampling_iteration == 0) {
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
}

void RigidBodyVehicle::set_wants_to_jump() {
    jump_state_.wants_to_jump_ = true;
    jump_state_.wants_to_jump_oversampled_ = true;
    jump_state_.jumping_counter_ = 0;
}

void RigidBodyVehicle::integrate_force(
    const VectorAtPosition<float, double, 3>& F,
    const PhysicsEngineConfig& cfg)
{
    rbi_.rbp_.integrate_impulse({
        .vector = F.vector * (cfg.dt / cfg.oversampling),
        .position = F.position});
    // if (float len = sum(squared(F.vector)); len > 1e-12) {
    //     auto location = TransformationMatrix<float, double, 3>::identity();
    //     location.t() = F.position;
    //     location.R() *= std::sqrt(sum(squared(F.vector))) / (N * 100'000.f);
    //     g_beacons.push_back(Beacon{ .location = location });
    // }
}

void RigidBodyVehicle::integrate_force(
    const VectorAtPosition<float, double, 3>& F,
    const FixedArray<float, 3>& n,
    float damping,
    float friction,
    const PhysicsEngineConfig& cfg)
{
    integrate_force(F, cfg);
    if (damping != 0) {
        auto vn = n * dot0d(rbi_.rbp_.v_, n);
        auto vt = rbi_.rbp_.v_ - vn;
        rbi_.rbp_.v_ = (1 - damping) * vn + vt * (1 - friction);
        rbi_.L_ *= 1 - damping;
    }
    if (damping != 0) {
        auto an = n * dot0d(rbi_.a_, n);
        auto at = rbi_.a_ - an;
        rbi_.a_ = (1 - damping) * an + at * (1 - friction);
        rbi_.T_ *= 1 - damping;
    }
}

void RigidBodyVehicle::integrate_gravity(const FixedArray<float, 3>& g) {
    rbi_.integrate_gravity(g);
}

// Note that g_beacons is delayed by one frame.
// namespace Mlib { extern std::list<Beacon> g_beacons; }

void RigidBodyVehicle::collide_with_air(
    const PhysicsEngineConfig& cfg,
    std::list<std::unique_ptr<ContactInfo>>& contact_infos)
{
    for (auto& [rotor_id, rotor] : rotors_) {
        PowerIntent P = consume_rotor_surface_power(rotor_id);
        if (P.type == PowerIntentType::ACCELERATE_OR_BREAK) {
            auto abs_location = rotor->rotated_location(rbi_.rbp_.abs_transformation(), rbi_.rbp_.v_);
            // g_beacons.push_back(Beacon{ .location = abs_location, .resource_name = "flag_z" });
            integrate_force(
                VectorAtPosition<float, double, 3>{
                    .vector = z3_from_3x3(abs_location.R()) * P.power * rotor->power2lift,
                    .position = abs_location.t() },
                cfg);
        } else {
            set_base_angular_velocity(*rotor, 0.f, TireAngularVelocityChange::IDLE);
        }
        set_base_angular_velocity(*rotor, rotor->w, TireAngularVelocityChange::ACCELERATE);
        if (rotor->blades_rb != nullptr) {
            rotor->blades_rb->rbi_.rbp_.w_ = rotor->angular_velocity * z3_from_3x3(rotor->blades_rb->rbi_.rbp_.rotation_);
            auto T0 = rbi_.rbp_.abs_transformation();
            auto T1 = rotor->blades_rb->rbi_.rbp_.abs_transformation();
            contact_infos.push_back(std::make_unique<PointContactInfo2>(
                rbi_.rbp_,
                rotor->blades_rb->rbi_.rbp_,
                PointEqualityConstraint{
                    .p0 = T0.transform(rotor->vehicle_mount_0.casted<double>()),
                    .p1 = T1.transform(rotor->blades_mount_0.casted<double>()),
                    .beta = cfg.point_equality_beta}));
            contact_infos.push_back(std::make_unique<PointContactInfo2>(
                rbi_.rbp_,
                rotor->blades_rb->rbi_.rbp_,
                PointEqualityConstraint{
                    .p0 = T0.transform(rotor->vehicle_mount_1.casted<double>()),
                    .p1 = T1.transform(rotor->blades_mount_1.casted<double>()),
                    .beta = cfg.point_equality_beta}));
        }
    }
    auto rbp_orig = rbi_.rbp_;
    for (auto& [wing_id, wing] : wings_) {
        // Absolute location
        auto abs_location = wing->absolute_location(rbi_.rbp_.abs_transformation());
        // Relative velocity
        auto vel = dot(rbp_orig.velocity_at_position(abs_location.t()), abs_location.R());
        auto vel2 = squared(vel);
        auto lvel = std::sqrt(sum(squared(vel)));
        auto svel2 = lvel * vel;
        auto drag = -wing->drag_coefficients * svel2;
        float fac = wing->fac(lvel);
        integrate_force(
            VectorAtPosition<float, double, 3>{
                .vector = abs_location.rotate(
                    fac * FixedArray<float, 3>{
                        drag(0),
                        drag(1) - svel2(2) * wing->angle_of_attack * wing->angle_coefficient_yz + vel2(2) * wing->lift_coefficient,
                        drag(2) - svel2(2) * std::abs(wing->brake_angle) * wing->angle_coefficient_zz}),
                .position = abs_location.t() },
            cfg);
    }
    if (!std::isnan(fly_forward_state_.wants_to_fly_forward_factor_)) {
        auto dir = rbi_.rbp_.rotation_.column(1);
        dir -= dot0d(dir, gravity_direction) * gravity_direction;
        float l2 = sum(squared(dir));
        if (l2 > 1e-6) {
            integrate_force(
                VectorAtPosition<float, double, 3>{
                    .vector = - (fly_forward_state_.wants_to_fly_forward_factor_ /
                                 std::sqrt(l2)) *
                                dir,
                    .position = abs_com() },
                cfg);
        }
    }
}

void RigidBodyVehicle::advance_time(float dt, std::list<Beacon>* beacons)
{
    std::lock_guard lock{advance_time_mutex_};
    rbi_.rbp_.advance_time(dt);
    for (auto& t : tires_) {
        t.second.advance_time(dt);
    }
    for (auto& [_, e] : engines_) {
        e.advance_time(dt, abs_com());
    }
#ifdef COMPUTE_POWER
    float nrg = energy();
    if (!std::isnan(energy_old_)) {
        power_ = (nrg - energy_old_) / dt;
    }
    energy_old_ = nrg;
#endif
}

float RigidBodyVehicle::mass() const {
    return rbi_.rbp_.mass_;
}

FixedArray<double, 3> RigidBodyVehicle::abs_com() const {
    return rbi_.rbp_.abs_com_;
}

FixedArray<float, 3, 3> RigidBodyVehicle::abs_I() const {
    return rbi_.abs_I();
}

FixedArray<double, 3> RigidBodyVehicle::abs_grind_point() const {
    if (!grind_state_.grind_point_.has_value()) {
        throw std::runtime_error("Grind point is not set");
    }
    return rbi_.rbp_.transform_to_world_coordinates(grind_state_.grind_point_.value());
}

FixedArray<double, 3> RigidBodyVehicle::abs_target() const {
    return rbi_.rbp_.transform_to_world_coordinates(target_);
}

VectorAtPosition<float, double, 3> RigidBodyVehicle::abs_F(const VectorAtPosition<float, double, 3>& F) const {
    return {
        .vector = dot1d(rbi_.rbp_.rotation_, F.vector),
        .position = dot1d(rbi_.rbp_.rotation_.casted<double>(), F.position) + rbi_.rbp_.abs_position()};
}

FixedArray<float, 3> RigidBodyVehicle::velocity_at_position(const FixedArray<double, 3>& position) const {
    return rbi_.velocity_at_position(position);
}

void RigidBodyVehicle::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) {
    rbi_.set_pose(
        absolute_model_matrix.R(),
        absolute_model_matrix.t());
}

TransformationMatrix<float, double, 3> RigidBodyVehicle::get_new_absolute_model_matrix() const {
    std::lock_guard lock{advance_time_mutex_};
    return rbi_.rbp_.abs_transformation();
}

void RigidBodyVehicle::notify_destroyed(void* obj) {
    if (driver_ != nullptr) {
        driver_->notify_vehicle_destroyed();
        driver_ = nullptr;
    }
    if (rigid_bodies_ != nullptr) {
        rigid_bodies_->delete_rigid_body(this);
    }
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
    get_wing(id).angle_of_attack = angle;
}

void RigidBodyVehicle::set_wing_brake_angle(size_t id, float angle) {
    get_wing(id).brake_angle = angle;
}

FixedArray<float, 3, 3> RigidBodyVehicle::get_abs_tire_rotation_matrix(size_t id) const {
    if (auto t = tires_.find(id); t != tires_.end()) {
        return dot2d(rbi_.rbp_.rotation_, rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, t->second.angle_y));
    } else {
        return rbi_.rbp_.rotation_;
    }
}

FixedArray<float, 3> RigidBodyVehicle::get_abs_tire_z(size_t id) const {
    FixedArray<float, 3> z{tires_z_};
    if (auto t = tires_.find(id); t != tires_.end()) {
        z = dot1d(rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, t->second.angle_y), z);
    }
    z = dot1d(rbi_.rbp_.rotation_, z);
    return z;
}

float RigidBodyVehicle::get_tire_angular_velocity(size_t id) const {
    return get_tire(id).angular_velocity;
}

void RigidBodyVehicle::set_tire_angular_velocity(size_t id, float w, TireAngularVelocityChange ch) {
    set_base_angular_velocity(get_tire(id), w, ch);
}

void RigidBodyVehicle::set_base_angular_velocity(BaseRotor& base_rotor, float w, TireAngularVelocityChange ch) {
    base_rotor.angular_velocity = w;
    if (ch == TireAngularVelocityChange::OFF) {
        engines_.at(base_rotor.engine).notify_off();
    }
    if ((ch == TireAngularVelocityChange::IDLE) || (ch == TireAngularVelocityChange::BREAK)) {
        engines_.at(base_rotor.engine).notify_idle(w);
    }
    if (ch == TireAngularVelocityChange::ACCELERATE) {
        engines_.at(base_rotor.engine).notify_accelerate(w);
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

PowerIntent RigidBodyVehicle::consume_tire_surface_power(size_t id) {
    Tire& tire = get_tire(id);
    auto e = engines_.find(tire.engine);
    if (e == engines_.end()) {
        throw std::runtime_error("No engine with name \"" + tire.engine + "\" exists");
    }
    return e->second.consume_abs_surface_power(id, tire.angular_velocity);
}

PowerIntent RigidBodyVehicle::consume_rotor_surface_power(size_t id) {
    Rotor& rotor = get_rotor(id);
    auto e = engines_.find(rotor.engine);
    if (e == engines_.end()) {
        throw std::runtime_error("No engine with name \"" + rotor.engine + "\" exists");
    }
    return e->second.consume_abs_surface_power(id, rotor.angular_velocity);
}

void RigidBodyVehicle::set_surface_power(
    const std::string& engine_name,
    float surface_power,
    float delta_power)
{
    auto e = engines_.find(engine_name);
    if (e == engines_.end()) {
        throw std::runtime_error("No engine with name \"" + engine_name + "\" exists");
    }
    e->second.set_surface_power(
        revert_surface_power_state_.revert_surface_power_
            ? -surface_power
            : surface_power,
        delta_power);
}

float RigidBodyVehicle::get_tire_break_force(size_t id) const {
    return get_tire(id).brake_force;
}

FixedArray<double, 3> RigidBodyVehicle::get_abs_tire_contact_position(size_t id) const {
    const Tire& tire = get_tire(id);
    return rbi_.rbp_.transform_to_world_coordinates(
        get_tire(id).position +
        FixedArray<float, 3>{0.f, tire.shock_absorber_position - tire.radius, 0.f});
}

const Tire& RigidBodyVehicle::get_tire(size_t id) const {
    return const_cast<RigidBodyVehicle*>(this)->get_tire(id);
}

Tire& RigidBodyVehicle::get_tire(size_t id) {
    auto it = tires_.find(id);
    if (it == tires_.end()) {
        throw std::runtime_error("No tire with ID " + std::to_string(id) + " exists");
    }
    return it->second;
}

const Rotor& RigidBodyVehicle::get_rotor(size_t id) const {
    return const_cast<RigidBodyVehicle*>(this)->get_rotor(id);
}

Rotor& RigidBodyVehicle::get_rotor(size_t id) {
    auto it = rotors_.find(id);
    if (it == rotors_.end()) {
        throw std::runtime_error("No rotor with ID " + std::to_string(id) + " exists");
    }
    return *it->second;
}

const Wing& RigidBodyVehicle::get_wing(size_t id) const {
    return const_cast<RigidBodyVehicle*>(this)->get_wing(id);
}

Wing& RigidBodyVehicle::get_wing(size_t id) {
    auto it = wings_.find(id);
    if (it == wings_.end()) {
        throw std::runtime_error("No wing with ID " + std::to_string(id) + " exists");
    }
    return *it->second;
}

float RigidBodyVehicle::energy() const {
    return rbi_.energy();
}

const std::string& RigidBodyVehicle::name() const {
    return name_;
}

void RigidBodyVehicle::set_rigid_bodies(RigidBodies& rigid_bodies) {
    if (rigid_bodies_ != nullptr) {
        throw std::runtime_error("Rigid bodies already set");
    }
    rigid_bodies_ = &rigid_bodies;
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

void RigidBodyVehicle::write_status(std::ostream& ostr, StatusComponents log_components) const {
    if (log_components & StatusComponents::TIME) {
        static const std::chrono::steady_clock::time_point epoch_time = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
        int64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - epoch_time).count();
        ostr << "t: " << milliseconds << " ms" << std::endl;
    }
    // if (true) {
    //     static std::chrono::time_point time_v0 = std::chrono::steady_clock::time_point();
    //     static std::chrono::time_point time_v100 = std::chrono::steady_clock::time_point();
    //     float v = std::sqrt(sum(squared(rbi_.rbp_.v_)));
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
    //             ostr << "t 0-100: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_v100 - time_v0).count() / 1000.f << " s" << std::endl;
    //         } else if (time_v100 < time_v0) {
    //             ostr << "t 100-0: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_v0 - time_v100).count() / 1000.f << " s" << std::endl;
    //         }
    //     }
    // }
    // if (true) {
    //     float dt = 1.f / 60.f * s;
    //     static FixedArray<float, 3> old_velocity;
    //     auto a = std::sqrt(sum(squared((rbi_.rbp_.v_ - old_velocity)))) / dt;
    //     old_velocity = rbi_.rbp_.v_;
    //     ostr << "a: " << a / (meters / (s * s)) << " m/s^2" << std::endl;
    // }
    if (log_components & StatusComponents::SPEED) {
        ostr << "v: " << std::sqrt(sum(squared(rbi_.rbp_.v_))) / kph << " km/h" << std::endl;
    }
    if (log_components & StatusComponents::ACCELERATION) {
        ostr << "a: " << std::sqrt(sum(squared(rbi_.a_))) / (meters / (s * s)) << " m/s^2" << std::endl;
    }
    if (log_components & StatusComponents::ANGULAR_VELOCITY) {
        ostr << "w: " << std::sqrt(sum(squared(rbi_.rbp_.w_))) / (degrees / s) << " ??/s" << std::endl;
    }
    if (log_components & StatusComponents::WHEEL_ANGULAR_VELOCITY) {
        ostr << "wt: " << std::sqrt(sum(squared(rbi_.rbp_.v_))) / WHEEL_RADIUS / (radians / s) << " rad/s" << std::endl;
    }
    if (log_components & StatusComponents::DIAMETER) {
        // T = 2 PI r / v, T = 2 PI / w
        // r = v / w
        // r / r2 = v * a / (w * v^2) = a / (w * v)
        if (float w2 = sum(squared(rbi_.rbp_.w_)); w2 > squared(0.01f * degrees / s)) {
            ostr << "d: " << 2 * std::sqrt(sum(squared(rbi_.rbp_.v_)) / w2) << " m" << std::endl;
            ostr << "d / d2(g): " << gravity_magnitude / std::sqrt(w2 * sum(squared(rbi_.rbp_.v_))) << std::endl;
        } else {
            ostr << "d: undefined" << std::endl;
            ostr << "d / d2(g): undefined" << std::endl;
        }
    }
    if (log_components & StatusComponents::DIAMETER2) {
        // F = m * a = m v^2 / r
        // r = v^2 / a
        if (float a2 = sum(squared(rbi_.a_)); a2 > squared(0.01f * meters / (s * s))) {
            ostr << "d2: " << 2 * sum(squared(rbi_.rbp_.v_)) / std::sqrt(a2) / meters << " m" << std::endl;
        } else {
            ostr << "d2: undefined" << std::endl;
        }
        // Not implemented: https://de.wikipedia.org/wiki/Wendekreis_(Fahrzeug)
        // D = 2 L / sin(alpha)
    }
    if (log_components & StatusComponents::POSITION) {
        auto pos = rbi_.abs_position();
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
            ostr << "P: " << power_ / PS << " PS" << std::endl;
        }
#endif
    }
    if ((log_components & StatusComponents::DRIVER_NAME) && (driver_ != nullptr)) {
        ostr << "Driver: " << driver_->name() << std::endl;
    }
    for (const auto& o : collision_observers_) {
        auto c = std::dynamic_pointer_cast<StatusWriter>(o);
        if (c != nullptr) {
            c->write_status(ostr, log_components);
        }
    }
    {
        auto c = dynamic_cast<StatusWriter*>(damageable_);
        if (c != nullptr) {
            c->write_status(ostr, log_components);
        }
    }
}

RigidBodyAvatarController& RigidBodyVehicle::avatar_controller() {
    if (avatar_controller_ == nullptr) {
        throw std::runtime_error("Rigid body \"" + name() + "\" has no avatar controller");
    }
    return *avatar_controller_;
}

RigidBodyPlaneController& RigidBodyVehicle::plane_controller() {
    if (plane_controller_ == nullptr) {
        throw std::runtime_error("Rigid body \"" + name() + "\" has no plane controller");
    }
    return *plane_controller_;
}

RigidBodyVehicleController& RigidBodyVehicle::vehicle_controller() {
    if (vehicle_controller_ == nullptr) {
        throw std::runtime_error("Rigid body \"" + name() + "\" has no vehicle controller");
    }
    return *vehicle_controller_;
}
