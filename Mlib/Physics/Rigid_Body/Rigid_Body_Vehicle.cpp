#include "Rigid_Body_Vehicle.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Geographic_Coordinates.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Physics/Actuators/Base_Rotor.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Collision/Constraints.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Type.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Avatar_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <chrono>

static const float WHEEL_RADIUS = 0.25f;

using namespace Mlib;

RigidBodyVehicle::RigidBodyVehicle(
    RigidBodies& rigid_bodies,
    const RigidBodyIntegrator& rbi,
    const TransformationMatrix<double, 3>* geographic_mapping,
    const std::string& name)
: rigid_bodies_{ rigid_bodies },
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
  style_updater_{ nullptr },
  driver_{ nullptr },
  avatar_controller_{ nullptr},
  vehicle_controller_{ nullptr},
  vehicle_type_{ VehicleType::UNDEFINED },
  grinding_{ false },
  wants_to_grind_{ false },
  geographic_mapping_{ geographic_mapping }
{}

RigidBodyVehicle::~RigidBodyVehicle()
{}

void RigidBodyVehicle::reset_forces() {
    rbi_.reset_forces();
    for (auto& e : engines_) {
        e.second.reset_forces();
    }
    grinding_ = false;
    wants_to_grind_ = false;
}

void RigidBodyVehicle::integrate_force(
    const VectorAtPosition<float, 3>& F,
    const PhysicsEngineConfig& cfg)
{
    if (cfg.resolve_collision_type == ResolveCollisionType::PENALTY) {
        rbi_.integrate_force(F);
    } else if (cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
        rbi_.rbp_.integrate_impulse({
            .vector = F.vector * (cfg.dt / cfg.oversampling),
            .position = F.position});
    } else {
        throw std::runtime_error("Unknown resolve collision type in integrate_force");
    }
}

void RigidBodyVehicle::integrate_force(
    const VectorAtPosition<float, 3>& F,
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

// namespace Mlib { extern std::list<Beacon> g_beacons; }

void RigidBodyVehicle::collide_with_air(
    const PhysicsEngineConfig& cfg,
    std::list<std::unique_ptr<ContactInfo>>& contact_infos) {
    for (auto& r : rotors_) {
        PowerIntent P = consume_rotor_surface_power(r.first);
        if (P.type == PowerIntentType::ACCELERATE_OR_BREAK) {
            auto abs_location = r.second->rotated_location(rbi_.rbp_.abs_transformation(), rbi_.rbp_.v_);
            // g_beacons.push_back(Beacon{ .location = abs_location, .resource_name = "flag_z" });
            integrate_force(
                VectorAtPosition<float, 3>{
                    .vector = z3_from_3x3(abs_location.R()) * P.power * r.second->power2lift,
                    .position = abs_location.t() },
                cfg);
        } else {
            set_base_angular_velocity(*r.second, 0.f, TireAngularVelocityChange::IDLE);
        }
        set_base_angular_velocity(*r.second, float(2 * M_PI) * 400.f / 60.f, TireAngularVelocityChange::ACCELERATE);
        if (r.second->blades_rb != nullptr) {
            r.second->blades_rb->rbi_.rbp_.w_ = r.second->angular_velocity * z3_from_3x3(r.second->blades_rb->rbi_.rbp_.rotation_);
            auto T0 = rbi_.rbp_.abs_transformation();
            auto T1 = r.second->blades_rb->rbi_.rbp_.abs_transformation();
            contact_infos.push_back(std::make_unique<PointContactInfo2>(
                rbi_.rbp_,
                r.second->blades_rb->rbi_.rbp_,
                PointEqualityConstraint{
                    .p0 = T0.transform(r.second->vehicle_mount_0),
                    .p1 = T1.transform(r.second->blades_mount_0),
                    .beta = 0.5f}));
            contact_infos.push_back(std::make_unique<PointContactInfo2>(
                rbi_.rbp_,
                r.second->blades_rb->rbi_.rbp_,
                PointEqualityConstraint{
                    .p0 = T0.transform(r.second->vehicle_mount_1),
                    .p1 = T1.transform(r.second->blades_mount_1),
                    .beta = 0.5f}));
        }
    }
}

void RigidBodyVehicle::advance_time(
    float dt,
    float min_acceleration,
    float min_velocity,
    float min_angular_velocity,
    PhysicsType physics_type,
    ResolveCollisionType resolve_collision_type,
    float hand_brake_velocity,
    std::list<Beacon>* beacons)
{
    std::lock_guard lock{advance_time_mutex_};
    if (physics_type == PhysicsType::TRACKING_SPRINGS) {
        for (auto& t : tires_) {
            FixedArray<float, 3, 3> rotation = get_abs_tire_rotation_matrix(t.first);
            FixedArray<float, 3> position = get_abs_tire_contact_position(t.first);
            FixedArray<float, 3> power_axis = get_abs_tire_z(t.first);
            FixedArray<float, 3> velocity = velocity_at_position(position);
            float spring_constant = 1e4;
            float power_internal;
            float power_external;
            float moment;
            bool slipping;
            t.second.tracking_wheel.update_position(
                rotation,
                position,
                power_axis,
                velocity,
                spring_constant,
                dt,
                rbi_,
                power_internal,
                power_external,
                moment,
                slipping,
                beacons);
            // static float spower = 0;
            // spower = 0.99 * spower + 0.01 * power;
            // std::cerr << "rb force " << force << std::endl;
            float P = consume_tire_surface_power(t.first).power;
            // std::cerr << "P " << P << " Pi " << power_internal << " Pe " << power_external << " " << (P > power_internal) << std::endl;
            if (!std::isnan(P)) {
                float dx_max = 0.1f;
                float w_max = dx_max / (t.second.tracking_wheel.radius() * dt);
                // std::cerr << "dx " << dx << std::endl;
                if ((P != 0) && (std::abs(P) > -power_internal) && !slipping) {
                    float v = dot0d(velocity, power_axis);
                    if (sign(P) != sign(v) && std::abs(v) > hand_brake_velocity) {
                        t.second.tracking_wheel.set_w(0);
                    } else if (P > 0) {
                        t.second.tracking_wheel.set_w(std::max(-w_max, t.second.tracking_wheel.w() - 0.5f));
                    } else if (P < 0) {
                        t.second.tracking_wheel.set_w(std::min(w_max, t.second.tracking_wheel.w() + 0.5f));
                    }
                } else {
                    if (moment < 0) {
                        t.second.tracking_wheel.set_w(std::max(-w_max, t.second.tracking_wheel.w() - 0.5f));
                    } else if (moment > 0) {
                        t.second.tracking_wheel.set_w(std::min(w_max, t.second.tracking_wheel.w() + 0.5f));
                    }
                }
            }
        }
    }
    if (resolve_collision_type == ResolveCollisionType::PENALTY) {
        rbi_.advance_time(dt, min_acceleration, min_velocity, min_angular_velocity);
    } else {
        rbi_.rbp_.advance_time(dt);
    }
    for (auto& t : tires_) {
        t.second.advance_time(dt);
    }
    for (auto& e : engines_) {
        e.second.advance_time(dt, abs_com());
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

FixedArray<float, 3> RigidBodyVehicle::abs_com() const {
    return rbi_.rbp_.abs_com_;
}

FixedArray<float, 3, 3> RigidBodyVehicle::abs_I() const {
    return rbi_.abs_I();
}

FixedArray<float, 3> RigidBodyVehicle::abs_grind_point() const {
    if (!grind_point_.has_value()) {
        throw std::runtime_error("Grind point is not set");
    }
    return rbi_.rbp_.transform_to_world_coordinates(grind_point_.value());
}

FixedArray<float, 3> RigidBodyVehicle::abs_target() const {
    return rbi_.rbp_.transform_to_world_coordinates(target_);
}

VectorAtPosition<float, 3> RigidBodyVehicle::abs_F(const VectorAtPosition<float, 3>& F) const {
    return {
        .vector = dot1d(rbi_.rbp_.rotation_, F.vector),
        .position = dot1d(rbi_.rbp_.rotation_, F.position) + rbi_.rbp_.abs_position()};
}

FixedArray<float, 3> RigidBodyVehicle::velocity_at_position(const FixedArray<float, 3>& position) const {
    return rbi_.velocity_at_position(position);
}

void RigidBodyVehicle::set_absolute_model_matrix(const TransformationMatrix<float, 3>& absolute_model_matrix) {
    rbi_.set_pose(
        absolute_model_matrix.R(),
        absolute_model_matrix.t());
}

TransformationMatrix<float, 3> RigidBodyVehicle::get_new_absolute_model_matrix() const {
    std::lock_guard lock{advance_time_mutex_};
    return TransformationMatrix<float, 3>{rbi_.rbp_.rotation_, rbi_.abs_position()};
}

void RigidBodyVehicle::notify_destroyed(void* obj) {
    rigid_bodies_.delete_rigid_body(this);
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

void RigidBodyVehicle::set_surface_power(const std::string& engine_name, float surface_power, float delta_power) {
    auto e = engines_.find(engine_name);
    if (e == engines_.end()) {
        throw std::runtime_error("No engine with name \"" + engine_name + "\" exists");
    }
    e->second.set_surface_power(surface_power, delta_power);
}

float RigidBodyVehicle::get_tire_break_force(size_t id) const {
    return get_tire(id).break_force;
}

TrackingWheel& RigidBodyVehicle::get_tire_tracking_wheel(size_t id) {
    return get_tire(id).tracking_wheel;
}

FixedArray<float, 3> RigidBodyVehicle::get_abs_tire_contact_position(size_t id) const {
    return rbi_.rbp_.transform_to_world_coordinates(get_tire(id).position - FixedArray<float, 3>{0.f, -get_tire(id).radius, 0.f});
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

float RigidBodyVehicle::energy() const {
    return rbi_.energy();
}

const std::string& RigidBodyVehicle::name() const {
    return name_;
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
    if (log_components & StatusComponents::SPEED) {
        ostr << "v: " << std::sqrt(sum(squared(rbi_.rbp_.v_))) * 3.6 << " km/h" << std::endl;
    }
    if (log_components & StatusComponents::ACCELERATION) {
        ostr << "a: " << std::sqrt(sum(squared(rbi_.a_))) << " m/s^2" << std::endl;
    }
    if (log_components & StatusComponents::ANGULAR_VELOCITY) {
        ostr << "w: " << std::sqrt(sum(squared(rbi_.rbp_.w_))) * float(180 / M_PI) << " °/s" << std::endl;
    }
    if (log_components & StatusComponents::WHEEL_ANGULAR_VELOCITY) {
        ostr << "wt: " << std::sqrt(sum(squared(rbi_.rbp_.v_))) / WHEEL_RADIUS << " rad/s" << std::endl;
    }
    if (log_components & StatusComponents::DIAMETER) {
        // T = 2 PI r / v, T = 2 PI / w
        // r = v / w
        // r / r2 = v * a / (w * v^2) = a / (w * v)
        if (float w2 = sum(squared(rbi_.rbp_.w_)); w2 > 1e-2) {
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
        if (float a2 = sum(squared(rbi_.a_)); a2 > 1e-2) {
            ostr << "d2: " << 2 * sum(squared(rbi_.rbp_.v_)) / std::sqrt(a2) << " m" << std::endl;
        } else {
            ostr << "d2: undefined" << std::endl;
        }
        // Not implemented: https://de.wikipedia.org/wiki/Wendekreis_(Fahrzeug)
        // D = 2 L / sin(alpha)
    }
    if (log_components & StatusComponents::POSITION) {
        auto pos = rbi_.abs_position();
        ostr << "x: " << pos(0) << " m" << std::endl;
        ostr << "y: " << pos(1) << " m" << std::endl;
        ostr << "z: " << pos(2) << " m" << std::endl;
        if (geographic_mapping_ != nullptr) {
            auto gpos = geographic_mapping_->transform(pos.casted<double>());
            ostr << "lat: " << latitude_to_string(gpos(0)) << std::endl;
            ostr << "lon: " << longitude_to_string(gpos(1)) << std::endl;
            ostr << "height: " << height_to_string(gpos(2)) << std::endl;
        }
    }
    if (log_components & StatusComponents::ENERGY) {
        ostr << "E: " << energy() / 1e3 << " kJ" << std::endl;
#ifdef COMPUTE_POWER
        // ostr << "P: " << power_ / 1e3 << " W" << std::endl;
        if (!std::isnan(power_)) {
            ostr << "P: " << power_ * 0.00135962 << " PS" << std::endl;
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

RigidBodyVehicleController& RigidBodyVehicle::vehicle_controller() {
    if (vehicle_controller_ == nullptr) {
        throw std::runtime_error("Rigid body \"" + name() + "\" has no vehicle controller");
    }
    return *vehicle_controller_;
}

RigidBodyAvatarController& RigidBodyVehicle::avatar_controller() {
    if (avatar_controller_ == nullptr) {
        throw std::runtime_error("Rigid body \"" + name() + "\" has no avatar controller");
    }
    return *avatar_controller_;
}
