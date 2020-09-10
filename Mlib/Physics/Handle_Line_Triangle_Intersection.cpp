#include "Handle_Line_Triangle_Intersection.hpp"
#include <Mlib/Geometry/Intersection/Ray_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Collision_Observer.hpp>
#include <Mlib/Physics/Objects/Rigid_Body.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Power_To_Force.hpp>
#include <Mlib/Physics/Sat_Normals.hpp>

using namespace Mlib;

#pragma GCC push_options
#pragma GCC optimize ("O3")

HandleLineTriangleIntersection::HandleLineTriangleIntersection(const IntersectionScene& i)
: i_{i}
{
    intersects_ = line_intersects_triangle(
        i_.l1(0),
        i_.l1(1),
        i_.t0,
        intersection_point_);
}

void HandleLineTriangleIntersection::handle()
{
    assert_true(intersects_);
    CollisionType collision_type = CollisionType::REFLECT;
    bool abort = false;
    for(auto& c0 : i_.o0->collision_observers_) {
        c0->notify_collided(i_.o1->collision_observers_, collision_type, abort);
    }
    for(auto& c1 : i_.o1->collision_observers_) {
        c1->notify_collided(i_.o0->collision_observers_, collision_type, abort);
    }
    if (abort) {
        return;
    }
    if (collision_type == CollisionType::GO_THROUGH) {
        // do nothing
    } else if (collision_type == CollisionType::STICK_TOGETHER) {
        FixedArray<float, 3> v;
        if (i_.o0->mass() == INFINITY || i_.o1->mass() == INFINITY) {
            v = 0;
        } else {
            v = (i_.o0->rbi_.v_ * i_.o0->mass() + i_.o1->rbi_.v_ * i_.o1->mass()) / (i_.o0->mass() + i_.o1->mass());
        }
        auto a0 = (v - i_.o0->rbi_.v_) / i_.cfg.dt;
        auto a1 = (v - i_.o1->rbi_.v_) / i_.cfg.dt;
        if (i_.o0->mass() != INFINITY) {
            i_.o0->integrate_force({i_.o0->mass() * a0, intersection_point_});
        }
        if (i_.o1->mass() != INFINITY) {
            i_.o1->integrate_force({i_.o1->mass() * a1, intersection_point_});
        }
    } else if (collision_type == CollisionType::REFLECT) {
        // beacons.push_back(intersection_point);
        PlaneNd<float, 3> plane;
        if (!i_.lines_are_normals && i_.o0->mass() != INFINITY && i_.o1->mass() != INFINITY) {
            if (!i_.cfg.sat) {
                plane = PlaneNd{
                    i_.o1->abs_com() - i_.o0->abs_com(),
                    intersection_point_};
            } else {
                // n = -st.get_collision_normal(o1, o0);
                // n = st->get_collision_normal(o0, o1);
                float min_overlap0;
                PlaneNd<float, 3> plane0;
                float min_overlap1;
                PlaneNd<float, 3> plane1;
                i_.st.get_collision_plane(i_.o0, i_.o1, i_.mesh0, i_.mesh1, min_overlap0, plane0);
                i_.st.get_collision_plane(i_.o1, i_.o0, i_.mesh1, i_.mesh0, min_overlap1, plane1);
                if (min_overlap0 < 0) {
                    throw std::runtime_error("No overlap detected (0)");
                }
                if (min_overlap1 < 0) {
                    throw std::runtime_error("No overlap detected (1)");
                }
                if (min_overlap0 > i_.cfg.overlap_tolerance * min_overlap1) {
                    return;
                }
                plane = plane0;
            }
        } else {
            plane = i_.p0;
        }
        float dist;
        if (i_.lines_are_normals) {
            dist = -(dot0d(i_.l1(1), plane.normal_) + plane.intercept_);
            if (dist < 0) {
                if (i_.mesh0_two_sided) {
                    plane.intercept_ *= -1;
                    plane.normal_ *= -1;
                    dist *= -1;
                } else {
                    return;
                }
            }
        } else {
            if (i_.mesh0_two_sided && (dot0d(i_.o1->abs_com(), plane.normal_) + plane.intercept_ < 0)) {
                plane.intercept_ *= -1;
                plane.normal_ *= -1;
            }
            float dist_0 = dot0d(i_.l1(0), plane.normal_) + plane.intercept_;
            float dist_1 = dot0d(i_.l1(1), plane.normal_) + plane.intercept_;
            // smallest negative distance
            dist = -std::min(dist_0, dist_1);
        }
        assert_true((dist >= 0) || (std::abs(dist) < 1e-3));
        if (i_.tire_id != SIZE_MAX) {
            dist = std::max(0.f, dist - i_.cfg.wheel_penetration_depth - i_.o1->tires_.at(i_.tire_id).shock_absorber.position());
            // std::cerr << "pos " << i_.o1->tires_.at(i_.tire_id).shock_absorber.position() << std::endl;
        } else {
            dist = std::max(0.f, dist);
        }
        float frac0;
        float frac1;
        if (i_.o0->mass() == INFINITY) {
            frac0 = 0;
            frac1 = 1;
        } else if (i_.o1->mass() == INFINITY) {
            frac0 = 1;
            frac1 = 0;
        } else {
            frac0 = i_.o1->mass() / (i_.o0->mass() + i_.o1->mass());
            frac1 = 1 - frac0;
        }
        auto o11 = i_.o1->rbi_;
        o11.advance_time(
            i_.cfg.dt,
            i_.cfg.min_acceleration,
            i_.cfg.min_velocity,
            i_.cfg.min_angular_velocity);
        auto v11 = o11.velocity_at_position(intersection_point_);
        float outness = dot0d(plane.normal_, v11);
        assert_true(dist >= 0);
        float force_n0 = NAN;
        float force_n1 = NAN;
        {
            float fac = i_.cfg.outness_fac_interp(outness) * squared(std::min(0.25f, dist));
            if (frac0 != 0) {
                force_n0 = fac * frac0 * i_.o0->mass();
            }
            if (frac1 != 0) {
                force_n1 = fac * frac1 * i_.o1->mass();
                if (i_.tire_id != SIZE_MAX) {
                    i_.o1->tires_.at(i_.tire_id).shock_absorber.integrate_force(force_n1);
                }
            }
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
        FixedArray<float, 3> motor_force;
        if (i_.tire_id != SIZE_MAX && i_.o0->mass() == INFINITY && i_.o1->mass() != INFINITY) {
            FixedArray<float, 3> n3 = i_.o1->get_abs_tire_z(i_.tire_id);
            n3 -= plane.normal_ * dot0d(plane.normal_, n3);
            if (float len2 = sum(squared(n3)); len2 > 1e-12) {
                n3 /= std::sqrt(len2);
                float P = i_.o1->consume_tire_surface_power(i_.tire_id);
                auto sp = [&](float ft, float fn){return power_to_force_infinite_mass(
                    i_.cfg.break_accel,
                    i_.cfg.tangential_accel * ft,
                    i_.cfg.hand_break_velocity,
                    i_.cfg.stiction_coefficient * force_n1 * fn,
                    i_.cfg.friction_coefficient * force_n1 * fn,
                    i_.o1->max_velocity_,
                    n3,
                    P,
                    i_.o1->mass(),
                    v11 - plane.normal_ * dot0d(plane.normal_, v11),
                    i_.cfg.dt,
                    i_.cfg.avoid_burnout);};
                FixedArray<float, 3> target_force = sp(1, INFINITY);
                float best_dist2 = INFINITY;
                for(float tfac = 2; tfac > 0.05; tfac *= 0.9) {
                    // bool tire_sliding = o1->get_tire_sliding(tire_id);
                    FixedArray<float, 3> mf = sp(tfac, 1);
                    if (float bd2 = sum(squared(mf - target_force)); bd2 < best_dist2) {
                        motor_force = mf;
                        best_dist2 = bd2;
                    }
                    // std::cerr << std::sqrt(sum(squared(motor_force_t))) / i_.o1->mass() << std::endl;
                }
            } else {
                motor_force = 0;
            }
        } else {
            motor_force = 0;
        }
        if (frac0 != 0) {
            i_.o0->integrate_force(
                {-force_n0 * plane.normal_ - motor_force, intersection_point_},
                plane.normal_,
                i_.cfg.damping,
                i_.cfg.friction);
        }
        if (frac1 != 0) {
            i_.o1->integrate_force(
                {force_n1 * plane.normal_ + motor_force, intersection_point_},
                plane.normal_,
                i_.cfg.damping,
                i_.tire_id == SIZE_MAX ? i_.cfg.friction : 0);
        }
    } else {
        throw std::runtime_error("Unknown collision type");
    }
}

void HandleLineTriangleIntersection::handle(const IntersectionScene& i) {
    HandleLineTriangleIntersection hlti{i};
    if (hlti.intersects_) {
        hlti.handle();
    }
}

#pragma GCC pop_options
