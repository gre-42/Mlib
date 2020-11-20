#include "Handle_Line_Triangle_Intersection.hpp"
#include <Mlib/Geometry/Intersection/Ray_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Constraints.hpp>
#include <Mlib/Physics/Handle_Tire_Triangle_Intersection.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Power_To_Force.hpp>
#include <Mlib/Physics/Sat_Normals.hpp>

using namespace Mlib;

#pragma GCC push_options
#pragma GCC optimize ("O3")

void Mlib::handle_line_triangle_intersection(const IntersectionScene& c)
{
    FixedArray<float, 3> intersection_point;
    bool intersects = line_intersects_triangle(
        c.l1(0),
        c.l1(1),
        c.t0,
        intersection_point);
    if (!intersects) {
        return;
    }
    CollisionType collision_type = CollisionType::REFLECT;
    bool abort = false;
    for(auto& c0 : c.o0->collision_observers_) {
        c0->notify_collided(c.o1->collision_observers_, collision_type, abort);
    }
    for(auto& c1 : c.o1->collision_observers_) {
        c1->notify_collided(c.o0->collision_observers_, collision_type, abort);
    }
    if (abort) {
        return;
    }
    if (collision_type == CollisionType::GO_THROUGH) {
        // do nothing
    } else if (collision_type == CollisionType::STICK_TOGETHER) {
        FixedArray<float, 3> v;
        if (c.o0->mass() == INFINITY || c.o1->mass() == INFINITY) {
            v = 0;
        } else {
            v = (c.o0->rbi_.rbp_.v_ * c.o0->mass() + c.o1->rbi_.rbp_.v_ * c.o1->mass()) / (c.o0->mass() + c.o1->mass());
        }
        auto a0 = (v - c.o0->rbi_.rbp_.v_) / (c.cfg.dt / c.cfg.oversampling);
        auto a1 = (v - c.o1->rbi_.rbp_.v_) / (c.cfg.dt / c.cfg.oversampling);
        if (c.o0->mass() != INFINITY) {
            c.o0->integrate_force({c.o0->mass() * a0, intersection_point});
        }
        if (c.o1->mass() != INFINITY) {
            c.o1->integrate_force({c.o1->mass() * a1, intersection_point});
        }
    } else if (collision_type == CollisionType::REFLECT) {
        // c.beacons.push_back({.position = intersection_point});
        PlaneNd<float, 3> plane;
        if (!c.lines_are_normals && c.o0->mass() != INFINITY && c.o1->mass() != INFINITY) {
            if (!c.cfg.sat) {
                plane = PlaneNd{
                    c.o1->abs_com() - c.o0->abs_com(),
                    intersection_point};
            } else {
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
        if (c.lines_are_normals) {
            penetrating_id = 1;
            dist = -(dot0d(c.l1(1), plane.normal) + plane.intercept);
            if (dist < 0) {
                if (c.mesh0_two_sided) {
                    plane.intercept *= -1;
                    plane.normal *= -1;
                    dist *= -1;
                } else {
                    return;
                }
            }
        } else {
            if (c.mesh0_two_sided && (dot0d(c.o1->abs_com(), plane.normal) + plane.intercept < 0)) {
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
        // c.beacons.push_back({.position = c.l1(penetrating_id)});
        assert_true((dist >= 0) || (std::abs(dist) < 1e-3));
        if (c.tire_id != SIZE_MAX) {
            dist = std::max(0.f, dist - c.cfg.wheel_penetration_depth - c.o1->tires_.at(c.tire_id).shock_absorber.position());
            // std::cerr << "pos " << c.o1->tires_.at(c.tire_id).shock_absorber.position() << std::endl;
        } else {
            dist = std::max(0.f, dist);
        }
        float frac0;
        float frac1;
        if (c.o0->mass() == INFINITY) {
            frac0 = 0;
            frac1 = 1;
        } else if (c.o1->mass() == INFINITY) {
            frac0 = 1;
            frac1 = 0;
        } else {
            frac0 = c.o1->mass() / (c.o0->mass() + c.o1->mass());
            frac1 = 1 - frac0;
        }
        float force_n0 = NAN;
        float force_n1 = NAN;
        const NormalImpulse* normal_impulse = nullptr;
        if (c.cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
            if (c.o0->mass() != INFINITY) {
                ContactInfo2* ci = new ContactInfo2{
                    c.o1->rbi_.rbp_,
                    c.o0->rbi_.rbp_,
                    BoundedPlaneConstraint{
                        .constraint{
                            .normal_impulse{.normal = plane.normal},
                            .intercept = plane.intercept,
                            .slop = (c.tire_id != SIZE_MAX)
                                ? -c.cfg.wheel_penetration_depth
                                : 0,
                            .beta = c.cfg.contact_beta,
                            .beta2 = c.cfg.contact_beta2
                        },
                        .lambda_min = (c.o0->mass() * c.o1->mass()) / (c.o0->mass() + c.o1->mass()) * c.cfg.lambda_min / c.cfg.oversampling,
                        .lambda_max = 0},
                    c.l1(penetrating_id)};
                c.contact_infos.push_back(std::unique_ptr<ContactInfo>(ci));
                normal_impulse = &ci->normal_impulse();
            } else {
                if (c.tire_id == SIZE_MAX) {
                    ContactInfo1* ci = new ContactInfo1{
                        c.o1->rbi_.rbp_,
                        BoundedPlaneConstraint{
                            .constraint{
                                .normal_impulse{.normal = plane.normal},
                                .intercept = plane.intercept,
                                .beta = c.cfg.contact_beta,
                                .beta2 = c.cfg.contact_beta2
                            },
                            .lambda_min = c.o1->mass() * c.cfg.lambda_min / c.cfg.oversampling,
                            .lambda_max = 0},
                        c.l1(penetrating_id)};
                    c.contact_infos.push_back(std::unique_ptr<ContactInfo>(ci));
                    normal_impulse = &ci->normal_impulse();
                } else {
                    float sap = c.cfg.wheel_penetration_depth + dot0d(c.l1(penetrating_id) - intersection_point, plane.normal);
                    c.o1->tires_.at(c.tire_id).shock_absorber_position = -std::min(0.f, sap);
                    if (sap < 0) {
                        ShockAbsorberContactInfo1* ci = new ShockAbsorberContactInfo1{
                            c.o1->rbi_.rbp_,
                            BoundedShockAbsorberConstraint{
                                .constraint{
                                    .normal_impulse{.normal = plane.normal},
                                    .distance = sap,
                                    .Ks = c.o1->tires_.at(c.tire_id).sKs,
                                    .Ka = c.o1->tires_.at(c.tire_id).sKa
                                },
                                .lambda_min = c.o1->mass() * c.cfg.lambda_min / c.cfg.oversampling,
                                .lambda_max = 0},
                            intersection_point};
                        c.contact_infos.push_back(std::unique_ptr<ContactInfo>(ci));
                        normal_impulse = &ci->normal_impulse();
                    }
                }
            }
        } else if (c.cfg.resolve_collision_type == ResolveCollisionType::PENALTY) {
            float outness;
            {
                auto o11 = c.o1->rbi_;
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
                    force_n0 = fac * frac0 * c.o0->mass();
                }
                if (frac1 != 0) {
                    force_n1 = fac * frac1 * c.o1->mass();
                    if (c.tire_id != SIZE_MAX) {
                        c.o1->tires_.at(c.tire_id).shock_absorber.integrate_force(force_n1);
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
        FixedArray<float, 3> tangential_force;
        if (c.o0->mass() == INFINITY && c.o1->mass() != INFINITY) {
            FixedArray<float, 3> v10 = c.o1->velocity_at_position(intersection_point);
            FixedArray<float, 3> v3 = v10 - plane.normal * dot0d(plane.normal, v10);
            if (c.tire_id != SIZE_MAX) {
                FixedArray<float, 3> n3 = c.o1->get_abs_tire_z(c.tire_id);
                n3 -= plane.normal * dot0d(plane.normal, n3);
                if (float len2 = sum(squared(n3)); len2 > 1e-12) {
                    n3 /= std::sqrt(len2);
                    if (false && (c.cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES)) {
                        auto t = cross(n3, plane.normal);
                        t /= std::sqrt(sum(squared(t)));
                        c.contact_infos.push_back(std::unique_ptr<ContactInfo>(new FrictionContactInfo1{
                            c.o1->rbi_.rbp_,
                            *normal_impulse,
                            c.l1(penetrating_id),
                            c.cfg.stiction_coefficient,
                            c.cfg.friction_coefficient,
                            30.f / 3.6f * n3}));
                        // ci.solve(c.cfg.dt / c.cfg.oversampling);
                        // std::cerr << c.tire_id << " lambda_total " << ci.pc().lambda_total / (c.cfg.dt / c.cfg.oversampling) << " " << c.cfg.stiction_coefficient * force_n1 << std::endl;
                    }
                    if (c.cfg.physics_type == PhysicsType::BUILTIN) {
                        if (c.cfg.resolve_collision_type == ResolveCollisionType::PENALTY) {
                            tangential_force = handle_tire_triangle_intersection(
                                *c.o1,
                                v3,
                                n3,
                                plane.normal,
                                force_n1,
                                c.cfg,
                                c.tire_id);
                        } else if (c.cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
                            if (normal_impulse != nullptr) {
                                c.contact_infos.push_back(std::unique_ptr<ContactInfo>(new TireContactInfo1{
                                    FrictionContactInfo1{
                                        c.o1->rbi_.rbp_,
                                        *normal_impulse,
                                        c.o1->get_abs_tire_contact_position(c.tire_id),
                                        c.cfg.stiction_coefficient,
                                        c.cfg.friction_coefficient,
                                        fixed_nans<float, 3>()},
                                    *c.o1,
                                    c.tire_id,
                                    v3,
                                    n3,
                                    -dot0d(c.o1->get_velocity_at_tire_contact(plane.normal, c.tire_id), n3),
                                    c.o1->get_angular_velocity_at_tire(plane.normal, c.tire_id),
                                    c.cfg}));
                            }
                        } else {
                            throw std::runtime_error("Unknown collision type");
                        }
                        // std::cerr << "P " << P << " Pi " << power_internal << " Pe " << power_external << " " << (P > power_internal) << std::endl;
                    } else if (c.cfg.physics_type == PhysicsType::TRACKING_SPRINGS) {
                        TrackingWheel& tw = c.o1->get_tire_tracking_wheel(c.tire_id);
                        tw.notify_intersection(
                            c.o1->get_abs_tire_rotation_matrix(c.tire_id),
                            c.o1->get_abs_tire_contact_position(c.tire_id),
                            intersection_point,
                            plane.normal,
                            c.cfg.stiction_coefficient * force_n1,
                            c.cfg.friction_coefficient * force_n1);
                        tangential_force = 0;
                    } else if (c.cfg.physics_type == PhysicsType::VERSION1) {
                        float P = c.o1->consume_tire_surface_power(c.tire_id).power;
                        tangential_force = power_to_force_infinite_mass(
                            c.o1->get_tire_break_force(c.tire_id),
                            c.cfg.hand_break_velocity,
                            c.cfg.stiction_coefficient * force_n1,
                            c.cfg.friction_coefficient * force_n1,
                            c.o1->max_velocity_,
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
                        c.cfg.stiction_coefficient * force_n1,
                        c.cfg.friction_coefficient * force_n1,
                        v3,
                        c.cfg.alpha0);
                } else if (c.cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
                    c.contact_infos.push_back(std::unique_ptr<ContactInfo>(new FrictionContactInfo1{
                        c.o1->rbi_.rbp_,
                        *normal_impulse,
                        c.l1(penetrating_id),
                        c.cfg.stiction_coefficient,
                        c.cfg.friction_coefficient,
                        fixed_zeros<float, 3>()}));
                }
            }
        } else {
            if (c.cfg.resolve_collision_type == ResolveCollisionType::PENALTY) {
                tangential_force = 0;
            } else if (c.cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
                c.contact_infos.push_back(std::unique_ptr<ContactInfo>(new FrictionContactInfo2{
                    c.o1->rbi_.rbp_,
                    c.o0->rbi_.rbp_,
                    *normal_impulse,
                    c.l1(penetrating_id),
                    c.cfg.stiction_coefficient,
                    c.cfg.friction_coefficient,
                    fixed_zeros<float, 3>()}));
            }
        }
        // if (float lr = c.cfg.stiction_coefficient * force_n1; lr > 1e-12) {
        //     std::cerr << "f " << c.tire_id << " " << std::sqrt(sum(squared(tangential_force))) / lr << std::endl;
        // }
        if (c.cfg.resolve_collision_type == ResolveCollisionType::PENALTY) {
            if (frac0 != 0) {
                c.o0->integrate_force({-force_n0 * plane.normal - tangential_force, intersection_point});
            }
            if (frac1 != 0) {
                if (c.tire_id != SIZE_MAX) {
                    c.o1->integrate_force({force_n1 * plane.normal + tangential_force, c.o1->get_abs_tire_contact_position(c.tire_id)});
                } else {
                    c.o1->integrate_force({force_n1 * plane.normal + tangential_force, intersection_point});
                }
            }
        }
    } else {
        throw std::runtime_error("Unknown collision type");
    }
}

#pragma GCC pop_options
