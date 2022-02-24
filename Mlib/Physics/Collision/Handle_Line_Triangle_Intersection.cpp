#include "Handle_Line_Triangle_Intersection.hpp"
#include <Mlib/Geometry/Intersection/Ray_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Collision/Constraints.hpp>
#include <Mlib/Physics/Collision/Grind_Info.hpp>
#include <Mlib/Physics/Collision/Handle_Tire_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Power_To_Force.hpp>
#include <Mlib/Physics/Collision/Sat_Normals.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

void Mlib::handle_line_triangle_intersection(const IntersectionScene& c)
{
    FixedArray<float, 3> intersection_point;
    {
        float t;
        if (!line_intersects_triangle(
            c.l1(0),
            c.l1(1),
            c.t0,
            t,
            &intersection_point))
        {
            return;
        }
    }
    CollisionType collision_type = c.default_collision_type;
    bool abort = false;
    for (auto& c0 : c.o0.collision_observers_) {
        c0->notify_collided(intersection_point, c.o1, CollisionRole::PRIMARY, collision_type, abort);
    }
    for (auto& c1 : c.o1.collision_observers_) {
        c1->notify_collided(intersection_point, c.o0, CollisionRole::SECONDARY, collision_type, abort);
    }
    if (abort) {
        return;
    }
    if (collision_type == CollisionType::GO_THROUGH) {
        // do nothing
    } else if (collision_type == CollisionType::STICK_TOGETHER) {
        FixedArray<float, 3> v;
        if (c.o0.mass() == INFINITY || c.o1.mass() == INFINITY) {
            v = 0;
        } else {
            v = (c.o0.rbi_.rbp_.v_ * c.o0.mass() + c.o1.rbi_.rbp_.v_ * c.o1.mass()) / (c.o0.mass() + c.o1.mass());
        }
        auto a0 = (v - c.o0.rbi_.rbp_.v_) / (c.cfg.dt / c.cfg.oversampling);
        auto a1 = (v - c.o1.rbi_.rbp_.v_) / (c.cfg.dt / c.cfg.oversampling);
        if (c.o0.mass() != INFINITY) {
            c.o0.integrate_force({c.o0.mass() * a0, intersection_point}, c.cfg);
        }
        if (c.o1.mass() != INFINITY) {
            c.o1.integrate_force({c.o1.mass() * a1, intersection_point}, c.cfg);
        }
    } else if (collision_type == CollisionType::REFLECT) {
        // #############
        // # Alignment #
        // #############
        if ((c.mesh0_material & PhysicsMaterial::ALIGNMENT_PLANE) &&
            ((dot0d(c.p0.normal, c.o1.rbi_.rbp_.rotation_.column(1)) < c.cfg.alignment_cos) ||
              c.o1.grind_state_.wants_to_grind_ ||
              !std::isnan(c.o1.fly_forward_state_.wants_to_fly_forward_factor_)))
        {
            return;
        }
        if (c.mesh1_type == MeshType::ALIGNMENT_CONTACT) {
            if (c.o1.align_to_surface_state_.align_to_surface_relaxation_ != 0.f) {
                if (c.mesh0_material & PhysicsMaterial::ALIGNMENT_PLANE) {
                    if (!c.o1.align_to_surface_state_.touches_alignment_plane_ ||
                        (c.p0.normal(1) > c.o1.align_to_surface_state_.surface_normal_(1)))
                    {
                        c.o1.align_to_surface_state_.touches_alignment_plane_ = true;
                        c.o1.align_to_surface_state_.surface_normal_ = c.p0.normal;
                    }
                } else if (!c.o1.align_to_surface_state_.touches_alignment_plane_ &&
                    (// (dot0d(plane.normal, c.o1.rbi_.rbp_.rotation_.column(1)) > c.cfg.alignment_cos) &&
                    (any(isnan(c.o1.align_to_surface_state_.surface_normal_)) ||
                    (c.p0.normal(1) > c.o1.align_to_surface_state_.surface_normal_(1)))))
                    // (c.o1.wants_to_grind_ && (plane.normal(1) > c.o1.surface_normal_(1))) ||
                    // (!c.o1.wants_to_grind_ && (dot0d(plane.normal - c.o1.surface_normal_, c.o1.rbi_.rbp_.rotation_.column(1)) > 0.f))))
                {
                    c.o1.align_to_surface_state_.surface_normal_ = c.p0.normal;
                }
            }
            // if (c.beacons != nullptr) {
            //     c.beacons->push_back(Beacon::create(intersection_point, "beacon"));
            // }
            return;
        }
        // #######
        // # SAT #
        // #######
        bool sat_used = false;
        // if (c.beacons != nullptr) {
        //     c.beacons->push_back(Beacon::create(intersection_point, "beacon"));
        // }
        PlaneNd<float, 3> plane;
        if (!c.l1_is_normal && c.o0.mass() != INFINITY && c.o1.mass() != INFINITY) {
            if (!c.cfg.sat) {
                plane = PlaneNd{
                    c.o1.abs_com() - c.o0.abs_com(),
                    intersection_point};
            } else {
                sat_used = true;
                // n = -st.get_collision_normal(o1, o0);
                // n = st->get_collision_normal(o0, o1);
                float min_overlap0;
                PlaneNd<float, 3> plane0;
                float min_overlap1;
                PlaneNd<float, 3> plane1;
                c.st.get_collision_plane(c.o0, c.o1, c.mesh0, c.mesh1, min_overlap0, plane0);
                c.st.get_collision_plane(c.o1, c.o0, c.mesh1, c.mesh0, min_overlap1, plane1);
                if (min_overlap0 < 0) {
                    throw std::runtime_error("No overlap detected (0)");
                }
                if (min_overlap1 < 0) {
                    throw std::runtime_error("No overlap detected (1)");
                }
                if (min_overlap0 > c.cfg.overlap_tolerance * min_overlap1) {
                    return;
                }
                plane = plane0;
            }
        } else {
            plane = c.p0;
        }
        float dist;
        size_t penetrating_id;
        if (c.l1_is_normal) {
            penetrating_id = 1;
            dist = -(dot0d(c.l1(1), plane.normal) + plane.intercept);
            if (c.mesh0_material & PhysicsMaterial::TWO_SIDED) {
                if (dist < 0) {
                    plane.intercept *= -1;
                    plane.normal *= -1;
                    dist *= -1;
                }
            } else if (dist < 1e-6) {
                // Epsilon enables two overlapping one-sided planes with
                // opposing normals.
                return;
            }
        } else {
            if ((c.mesh0_material & PhysicsMaterial::TWO_SIDED) &&
                (dot0d(c.o1.abs_com(), plane.normal) + plane.intercept < 0))
            {
                plane.intercept *= -1;
                plane.normal *= -1;
            }
            float dist_0 = dot0d(c.l1(0), plane.normal) + plane.intercept;
            float dist_1 = dot0d(c.l1(1), plane.normal) + plane.intercept;
            // smallest negative distance
            // dist = -std::min(dist_0, dist_1);
            if (dist_0 < dist_1) {
                penetrating_id = 0;
                dist = -dist_0;
            } else {
                penetrating_id = 1;
                dist = -dist_1;
            }
        }
        // if (c.beacons != nullptr) {
        //     c.beacons->push_back(Beacon::create(c.l1(penetrating_id), "beacon"));
        // }
        if (dist < float{ -1e-3 }) {
            if (sat_used) {
                throw std::runtime_error(
                    "Line and triangle do not overlap. "
                    "Are the objects non-convex? Gap: " +
                    std::to_string(-dist));
            } else {
                throw std::runtime_error(
                    "Line and triangle do not overlap. "
                    "Gap: " +
                    std::to_string(-dist));
            }
        }
        if (c.tire_id1 != SIZE_MAX) {
            dist = std::max(0.f, dist - c.cfg.wheel_penetration_depth - c.o1.tires_.at(c.tire_id1).shock_absorber.position());
            // std::cerr << "pos " << c.o1.tires_.at(c.tire_id1).shock_absorber.position() << std::endl;
        } else {
            dist = std::max(0.f, dist);
        }
        // ################
        // # Normal force #
        // ################
        float frac0;
        float frac1;
        if (c.o0.mass() == INFINITY) {
            frac0 = 0;
            frac1 = 1;
        } else if (c.o1.mass() == INFINITY) {
            frac0 = 1;
            frac1 = 0;
        } else {
            frac0 = c.o1.mass() / (c.o0.mass() + c.o1.mass());
            frac1 = 1 - frac0;
        }
        float force_n0 = NAN;
        float force_n1 = NAN;
        const NormalImpulse* normal_impulse = nullptr;
        if (c.cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
            if (c.o0.mass() != INFINITY) {
                auto ci = std::make_unique<NormalContactInfo2>(
                    c.o1.rbi_.rbp_,
                    c.o0.rbi_.rbp_,
                    BoundedPlaneInequalityConstraint{
                        .constraint{
                            .normal_impulse{.normal = plane.normal},
                            .intercept = plane.intercept,
                            .slop = (c.tire_id1 != SIZE_MAX)
                                ? 0.001f
                                : 0.f,
                            .beta = c.cfg.plane_inequality_beta
                        },
                        .lambda_min = (c.o0.mass() * c.o1.mass()) / (c.o0.mass() + c.o1.mass()) * c.cfg.lambda_min / c.cfg.oversampling,
                        .lambda_max = 0},
                    // c.l1(penetrating_id)};
                    c.tire_id1 != SIZE_MAX ? c.o1.get_abs_tire_contact_position(c.tire_id1) : c.l1(penetrating_id),
                    [c, plane](float lambda_final){
                        for (auto& c0 : c.o0.collision_observers_) {
                            c0->notify_impact(c.o1, CollisionRole::PRIMARY, plane.normal, lambda_final, c.base_log);
                        }
                        for (auto& c1 : c.o1.collision_observers_) {
                            c1->notify_impact(c.o0, CollisionRole::SECONDARY, plane.normal, lambda_final, c.base_log);
                        }
                    });
                normal_impulse = &ci->normal_impulse();
                c.contact_infos.push_back(std::move(ci));
            } else {
                if (c.tire_id1 == SIZE_MAX) {
                    auto ci = std::make_unique<NormalContactInfo1>(
                        c.o1.rbi_.rbp_,
                        BoundedPlaneInequalityConstraint{
                            .constraint{
                                .normal_impulse{.normal = plane.normal},
                                .intercept = plane.intercept,
                                .slop = 0.001f,
                                .beta = c.cfg.plane_inequality_beta
                            },
                            .lambda_min = c.o1.mass() * c.cfg.lambda_min / c.cfg.oversampling,
                            .lambda_max = 0},
                        c.l1(penetrating_id));
                    normal_impulse = &ci->normal_impulse();
                    c.contact_infos.push_back(std::move(ci));
                } else {
                    float penetration_depth = dot0d(c.l1(penetrating_id) - intersection_point, plane.normal);
                    if (c.o1.jump_state_.wants_to_jump_oversampled_ && !c.o1.grind_state_.grinding_ && !(c.mesh0_material & PhysicsMaterial::ALIGNMENT_PLANE)) {
                        penetration_depth -= 0.25f;
                    }
                    float sap = std::min(0.05f, c.cfg.wheel_penetration_depth + penetration_depth);
                    c.o1.tires_.at(c.tire_id1).shock_absorber_position = -sap;
                    auto ci = std::make_unique<ShockAbsorberContactInfo1>(
                        c.o1.rbi_.rbp_,
                        BoundedShockAbsorberConstraint{
                            .constraint{
                                .normal_impulse{.normal = plane.normal},
                                .distance = sap,
                                .Ks = c.o1.tires_.at(c.tire_id1).sKs,
                                .Ka = c.o1.tires_.at(c.tire_id1).sKa
                            },
                            .lambda_min = c.o1.mass() * c.cfg.lambda_min / c.cfg.oversampling,
                            .lambda_max = 0},
                        intersection_point);
                    normal_impulse = &ci->normal_impulse();
                    c.contact_infos.push_back(std::move(ci));
                }
            }
        } else if (c.cfg.resolve_collision_type == ResolveCollisionType::PENALTY) {
            float outness;
            {
                auto o11 = c.o1.rbi_;
                o11.advance_time(
                    c.cfg.dt / c.cfg.oversampling,
                    c.cfg.min_acceleration,
                    c.cfg.min_velocity,
                    c.cfg.min_angular_velocity);
                auto v11 = o11.velocity_at_position(intersection_point);
                outness = dot0d(plane.normal, v11);
            }
            assert_true(dist >= 0);
            {
                float fac = c.cfg.outness_fac_interp(outness) * squared(std::min(0.25f, dist));
                if (frac0 != 0) {
                    force_n0 = fac * frac0 * c.o0.mass();
                }
                if (frac1 != 0) {
                    force_n1 = fac * frac1 * c.o1.mass();
                    if (c.tire_id1 != SIZE_MAX) {
                        c.o1.tires_.at(c.tire_id1).shock_absorber.integrate_force(force_n1);
                    }
                }
            }
        } else {
            throw std::runtime_error("Unknown resolve collision type in handle_line_triangle_intersection: " + std::to_string(int(c.cfg.resolve_collision_type)));
        }
        // if (outness < -10) {
        //     fac = 1.5e3;
        // } else if (outness < 0.05) {
        //     fac = 1e3;
        // } else if (outness < 0.2) {
        //     fac = 1e1;
        // } else {
        //     fac = 1;
        // }
        // ####################
        // # Tangential force #
        // ####################
        FixedArray<float, 3> tangential_force;
        bool align = (c.mesh0_material & PhysicsMaterial::ALIGNMENT_PLANE);
        if (c.o0.mass() == INFINITY && c.o1.mass() != INFINITY) {
            FixedArray<float, 3> v10 = c.o1.velocity_at_position(intersection_point);
            FixedArray<float, 3> v3 = v10 - plane.normal * dot0d(plane.normal, v10);
            if (c.tire_id1 != SIZE_MAX) {
                FixedArray<float, 3> n3 = c.o1.get_abs_tire_z(c.tire_id1);
                n3 -= plane.normal * dot0d(plane.normal, n3);
                if (float len2 = sum(squared(n3)); len2 > 1e-12) {
                    n3 /= std::sqrt(len2);
                    if (false && (c.cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES)) {
                        auto t = cross(n3, plane.normal);
                        t /= std::sqrt(sum(squared(t)));
                        c.contact_infos.push_back(std::unique_ptr<ContactInfo>(new FrictionContactInfo1{
                            c.o1.rbi_.rbp_,
                            *normal_impulse,
                            c.l1(penetrating_id),
                            align ? 0.f : c.o1.tires_.at(c.tire_id1).stiction_coefficient(-force_n1),
                            align ? 0.f : c.o1.tires_.at(c.tire_id1).friction_coefficient(-force_n1),
                            30.f / 3.6f * n3}));
                        // ci.solve(c.cfg.dt / c.cfg.oversampling);
                        // std::cerr << c.tire_id1 << " lambda_total " << ci.pc().lambda_total / (c.cfg.dt / c.cfg.oversampling) << " " << c.cfg.stiction_coefficient * force_n1 << std::endl;
                    }
                    if (c.cfg.physics_type == PhysicsType::BUILTIN) {
                        FixedArray<float, 3> contact_position = c.o1.get_abs_tire_contact_position(c.tire_id1);
                        FixedArray<float, 3> v_street = c.o0.velocity_at_position(contact_position);
                        FixedArray<float, 3> vc_street = c.o0.velocity_at_position(c.o1.abs_com());
                        vc_street -= plane.normal * dot0d(plane.normal, vc_street);
                        if (c.cfg.resolve_collision_type == ResolveCollisionType::PENALTY) {
                            tangential_force = handle_tire_triangle_intersection(
                                c.o1,
                                v_street,
                                vc_street,
                                v3,
                                n3,
                                plane.normal,
                                align ? 0.f : c.o1.tires_.at(c.tire_id1).stiction_coefficient(-force_n1) * force_n1,
                                align ? 0.f : c.o1.tires_.at(c.tire_id1).friction_coefficient(-force_n1) * force_n1,
                                c.cfg,
                                c.tire_id1);
                        } else if (c.cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
                            if (normal_impulse != nullptr) {
                                FixedArray<float, 3> vc = c.o1.rbi_.rbp_.v_;
                                vc -= plane.normal * dot0d(plane.normal, vc);
                                FixedArray<float, 3> contact_position = c.o1.get_abs_tire_contact_position(c.tire_id1);
                                FixedArray<float, 3> v_street = c.o0.velocity_at_position(contact_position);
                                FixedArray<float, 3> vc_street = c.o0.velocity_at_position(c.o1.abs_com());
                                c.contact_infos.push_back(std::unique_ptr<ContactInfo>(new TireContactInfo1{
                                    FrictionContactInfo1{
                                        c.o1.rbi_.rbp_,
                                        *normal_impulse,
                                        contact_position,
                                        NAN, // clamping handled by "TireContactInfo1" // c.o1.tires_.at(c.tire_id1).stiction_coefficient(-force_n1),
                                        NAN, // clamping handled by "TireContactInfo1" // c.o1.tires_.at(c.tire_id1).friction_coefficient(-force_n1),
                                        v_street},
                                    c.o1,
                                    c.tire_id1,
                                    vc_street,
                                    vc,
                                    n3,
                                    -dot0d(c.o1.get_velocity_at_tire_contact(plane.normal, c.tire_id1) - v_street, n3),
                                    c.cfg}));
                                // if (c.beacons != nullptr) {
                                //     c.beacons->push_back(Beacon::create(contact_position, "beacon"));
                                // }
                            }
                        } else {
                            throw std::runtime_error("Unknown collision type");
                        }
                        // std::cerr << "P " << P << " Pi " << power_internal << " Pe " << power_external << " " << (P > power_internal) << std::endl;
                    } else if (c.cfg.physics_type == PhysicsType::TRACKING_SPRINGS) {
                        TrackingWheel& tw = c.o1.get_tire_tracking_wheel(c.tire_id1);
                        tw.notify_intersection(
                            c.o1.get_abs_tire_rotation_matrix(c.tire_id1),
                            c.o1.get_abs_tire_contact_position(c.tire_id1),
                            intersection_point,
                            plane.normal,
                            align ? 0.f : c.o1.tires_.at(c.tire_id1).stiction_coefficient(-force_n1) * force_n1,
                            align ? 0.f : c.o1.tires_.at(c.tire_id1).friction_coefficient(-force_n1) * force_n1);
                        tangential_force = 0;
                    } else if (c.cfg.physics_type == PhysicsType::VERSION1) {
                        float P = c.o1.consume_tire_surface_power(c.tire_id1).power;
                        tangential_force = power_to_force_infinite_mass(
                            c.o1.get_tire_break_force(c.tire_id1),
                            c.cfg.hand_brake_velocity,
                            align ? 0.f : c.o1.tires_.at(c.tire_id1).stiction_coefficient(-force_n1) * force_n1,
                            align ? 0.f : c.o1.tires_.at(c.tire_id1).friction_coefficient(-force_n1) * force_n1,
                            c.o1.max_velocity_,
                            n3,
                            P,
                            v3,
                            c.cfg.dt / c.cfg.oversampling,
                            c.cfg.alpha0,
                            c.cfg.avoid_burnout);
                    } else {
                        throw std::runtime_error("Unknown physics type");
                    }
                } else {
                    tangential_force = 0;
                }
            } else {
                if (c.cfg.resolve_collision_type == ResolveCollisionType::PENALTY) {
                    tangential_force = friction_force_infinite_mass(
                        align ? 0.f : c.cfg.stiction_coefficient * force_n1,
                        align ? 0.f : c.cfg.friction_coefficient * force_n1,
                        v3,
                        c.cfg.alpha0);
                } else if (c.cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
                    FixedArray<float, 3> contact_position = c.l1(penetrating_id);
                    c.contact_infos.push_back(std::unique_ptr<ContactInfo>(new FrictionContactInfo1{
                        c.o1.rbi_.rbp_,
                        *normal_impulse,
                        contact_position,
                        align ? 0.f : c.cfg.stiction_coefficient,
                        align ? 0.f : c.cfg.friction_coefficient,
                        c.o0.velocity_at_position(contact_position)}));
                }
            }
        } else {
            if (c.cfg.resolve_collision_type == ResolveCollisionType::PENALTY) {
                tangential_force = 0;
            } else if (c.cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
                c.contact_infos.push_back(std::unique_ptr<ContactInfo>(new FrictionContactInfo2{
                    c.o1.rbi_.rbp_,
                    c.o0.rbi_.rbp_,
                    *normal_impulse,
                    c.l1(penetrating_id),
                    align ? 0.f : c.cfg.stiction_coefficient,
                    align ? 0.f : c.cfg.friction_coefficient,
                    fixed_zeros<float, 3>()}));
            }
        }
        // if (float lr = c.cfg.stiction_coefficient * force_n1; lr > 1e-12) {
        //     std::cerr << "f " << c.tire_id1 << " " << std::sqrt(sum(squared(tangential_force))) / lr << std::endl;
        // }
        if (c.cfg.resolve_collision_type == ResolveCollisionType::PENALTY) {
            if (frac0 != 0) {
                c.o0.rbi_.integrate_force({-force_n0 * plane.normal - tangential_force, intersection_point});
            }
            if (frac1 != 0) {
                if (c.tire_id1 != SIZE_MAX) {
                    c.o1.rbi_.integrate_force({force_n1 * plane.normal + tangential_force, c.o1.get_abs_tire_contact_position(c.tire_id1)});
                } else {
                    c.o1.rbi_.integrate_force({force_n1 * plane.normal + tangential_force, intersection_point});
                }
            }
        }
    } else if (collision_type == CollisionType::GRIND) {
        if (!c.o0.grind_state_.wants_to_grind_) {
            return;
        }
        FixedArray<float, 3> d3 = intersection_point - c.o0.abs_grind_point();
        FixedArray<float, 3> rail_direction = c.l1(1) - c.l1(0);
        float rail_len2 = sum(squared(rail_direction));
        if (rail_len2 < 1e-12) {
            throw std::runtime_error("Grind rail too short");
        }
        rail_direction /= std::sqrt(rail_len2);
        if (std::abs(dot0d(rail_direction, triangle_normal(c.t0))) < c.cfg.max_grind_cos) {
            return;
        }
        bool direction_ok = false;
        if (!any(isnan(c.o0.grind_state_.grind_direction_))) {
            float vl = std::abs(dot0d(c.o0.grind_state_.grind_direction_, rail_direction));
            if (vl > c.cfg.continuos_grind_projected_velocity_threshold) {
                direction_ok = true;
            }
        }
        if (!direction_ok) {
            if (c.o0.grind_state_.wants_to_grind_counter_ > c.cfg.nframes_straight_grind) {
                float v_len2 = sum(squared(c.o0.rbi_.rbp_.v_));
                if (v_len2 > c.cfg.continuos_grind_velocity_threshold) {
                    float vl = std::abs(dot0d(c.o0.rbi_.rbp_.v_, rail_direction) / std::sqrt(v_len2));
                    if (vl < c.cfg.continuos_grind_projected_velocity_threshold) {
                        return;
                    }
                }
            }
        }
        GrindInfo gi{
            .squared_distance = sum(squared(d3)),
            .intersection_point = intersection_point,
            .rail_direction = rail_direction,
            .rail_rb = &c.o1 };
        auto res = c.grind_infos.insert({ &c.o0, gi });
        if (!res.second) {
            if (gi.squared_distance < res.first->second.squared_distance) {
                res.first->second = gi;
            }
        }
    } else {
        throw std::runtime_error("Unknown collision type");
    }
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
