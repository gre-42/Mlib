#include "Handle_Line_Triangle_Intersection.hpp"
#include <Mlib/Geometry/Intersection/Ray_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Collision_History.hpp>
#include <Mlib/Physics/Collision/Grind_Info.hpp>
#include <Mlib/Physics/Collision/Record/Handle_Reflection.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

void Mlib::handle_line_triangle_intersection(const IntersectionScene& c)
{
    FixedArray<double, 3> intersection_point;
    double t;
    if (!line_intersects_triangle(
        c.l1(0),
        c.l1(1),
        c.t0,
        t,
        &intersection_point))
    {
        return;
    }
    if (any(c.mesh1_material & PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT)) {
        IntersectionSceneAndContact cc{
            .scene = c,
            .ray_t = t,
            .intersection_point = intersection_point};
        auto res = c.history.raycast_intersections.insert({ &c.l1, cc });
        if (!res.second) {
            if (cc.ray_t < res.first->second.ray_t) {
                c.history.raycast_intersections.erase(res.first);
                c.history.raycast_intersections.insert({ &c.l1, cc });
            }
        }
    } else {
        handle_line_triangle_intersection(c, intersection_point);
    }
}

void Mlib::handle_line_triangle_intersection(
    const IntersectionScene& c,
    const FixedArray<double, 3>& intersection_point)
{
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
        auto a0 = (v - c.o0.rbi_.rbp_.v_) / (c.history.cfg.dt / c.history.cfg.oversampling);
        auto a1 = (v - c.o1.rbi_.rbp_.v_) / (c.history.cfg.dt / c.history.cfg.oversampling);
        if (c.o0.mass() != INFINITY) {
            c.o0.integrate_force({c.o0.mass() * a0, intersection_point}, c.history.cfg);
        }
        if (c.o1.mass() != INFINITY) {
            c.o1.integrate_force({c.o1.mass() * a1, intersection_point}, c.history.cfg);
        }
    } else if (collision_type == CollisionType::REFLECT) {
        handle_reflection(c, intersection_point);
    } else if (collision_type == CollisionType::GRIND) {
        if (!c.o0.grind_state_.wants_to_grind_) {
            return;
        }
        FixedArray<float, 3> d3 = (intersection_point - c.o0.abs_grind_point()).casted<float>();
        FixedArray<double, 3> rail_direction = c.l1(1) - c.l1(0);
        float rail_len2 = sum(squared(rail_direction));
        if (rail_len2 < 1e-12) {
            throw std::runtime_error("Grind rail too short");
        }
        rail_direction /= std::sqrt(rail_len2);
        if (std::abs(dot0d(rail_direction, triangle_normal(c.t0))) < c.history.cfg.max_grind_cos) {
            return;
        }
        bool direction_ok = false;
        if (!any(Mlib::isnan(c.o0.grind_state_.grind_direction_))) {
            float vl = std::abs(dot0d(c.o0.grind_state_.grind_direction_, rail_direction.casted<float>()));
            if (vl > c.history.cfg.continuos_grind_cos_threshold) {
                direction_ok = true;
            }
        }
        if (!direction_ok) {
            if (c.o0.grind_state_.wants_to_grind_counter_ > c.history.cfg.nframes_straight_grind) {
                float v_len2 = sum(squared(c.o0.rbi_.rbp_.v_));
                if (v_len2 > squared(c.history.cfg.continuos_grind_velocity_threshold)) {
                    float vl = std::abs(dot0d(c.o0.rbi_.rbp_.v_, rail_direction.casted<float>()) / std::sqrt(v_len2));
                    if (vl < c.history.cfg.continuos_grind_cos_threshold) {
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
        auto res = c.history.grind_infos.insert({ &c.o0, gi });
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