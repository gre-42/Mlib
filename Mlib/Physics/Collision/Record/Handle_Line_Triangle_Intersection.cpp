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
#include <Mlib/Physics/Smoke_Generation/Contact_Smoke_Generator.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

void Mlib::handle_line_triangle_intersection(const IntersectionScene& c)
{
    if (&c.o0 == &c.o1) {
        THROW_OR_ABORT("Collision of identical objects");
    }
    if ((c.l1 == nullptr) == (c.r1 == nullptr)) {
        THROW_OR_ABORT("handle_line_triangle_intersection: Not exactly one of l1/r1 are set");
    }
    const auto& L1 = (c.l1 != nullptr) ? *c.l1->line : *c.r1->edge;
#define l1 DO_NOT_USE_ME
#define r1 DO_NOT_USE_ME
    FixedArray<double, 3> intersection_point;
    double t;
    if (!line_intersects_triangle(
        L1(0),
        L1(1),
        c.t0.triangle,
        t,
        &intersection_point))
    {
        return;
    }
    if (any(c.mesh1_material & PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT) &&
        !c.l1_is_normal)
    {
        THROW_OR_ABORT("Unexpected c.l1_is_normal value");
    }
    if (c.l1_is_normal) {
        IntersectionSceneAndContact cc{
            .scene = c,
            .ray_t = t,
            .intersection_point = intersection_point};
        auto res = c.history.raycast_intersections.insert({ &L1, cc });
        if (!res.second) {
            if (cc.ray_t < res.first->second.ray_t) {
                c.history.raycast_intersections.erase(res.first);
                c.history.raycast_intersections.insert({ &L1, cc });
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
#undef l1
#undef r1
    if ((c.l1 == nullptr) == (c.r1 == nullptr)) {
        THROW_OR_ABORT("handle_line_triangle_intersection: Not exactly one of l1/e1 are set");
    }
    const auto& L1 = (c.l1 != nullptr) ? *c.l1->line : *c.r1->edge;
#define l1 DO_NOT_USE_ME
#define r1 DO_NOT_USE_ME
    CollisionType collision_type = c.default_collision_type;
    bool abort = false;
    for (auto& c0 : c.o0.collision_observers_) {
        c0->notify_collided(intersection_point, c.o1, CollisionRole::PRIMARY, collision_type, abort);
    }
    for (auto& c1 : c.o1.collision_observers_) {
        c1->notify_collided(intersection_point, c.o0, CollisionRole::SECONDARY, collision_type, abort);
    }
    auto* scinfo = c.history.scdb.notify_contact(intersection_point, c);
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
        auto a0 = (v - c.o0.rbi_.rbp_.v_) / (c.history.cfg.dt / (float)c.history.cfg.oversampling);
        auto a1 = (v - c.o1.rbi_.rbp_.v_) / (c.history.cfg.dt / (float)c.history.cfg.oversampling);
        if (c.o0.mass() != INFINITY) {
            c.o0.integrate_force({c.o0.mass() * a0, intersection_point}, c.history.cfg);
        }
        if (c.o1.mass() != INFINITY) {
            c.o1.integrate_force({c.o1.mass() * a1, intersection_point}, c.history.cfg);
        }
    } else if (collision_type == CollisionType::REFLECT) {
        handle_reflection(
            c,
            intersection_point,
            scinfo == nullptr
                ? 1.f
                : scinfo->surface_stiction_factor);
    } else if (collision_type == CollisionType::GRIND) {
        if (!c.o0.grind_state_.wants_to_grind_) {
            return;
        }
        FixedArray<float, 3> d3 = (intersection_point - c.o0.abs_grind_point()).casted<float>();
        FixedArray<double, 3> rail_direction = L1(1) - L1(0);
        double rail_len2 = sum(squared(rail_direction));
        if (rail_len2 < 1e-12) {
            THROW_OR_ABORT("Grind rail too short");
        }
        rail_direction /= std::sqrt(rail_len2);
        if (std::abs(dot0d(rail_direction, c.t0.plane.normal)) < c.history.cfg.max_grind_cos) {
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
        THROW_OR_ABORT("Unknown collision type");
    }
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
