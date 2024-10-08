#include "Handle_Line_Triangle_Intersection.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Grind_Info.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Handle_Reflection.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Domain.hpp>
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
    if ((c.q0 == nullptr) == (c.t0 == nullptr)) {
        THROW_OR_ABORT("handle_line_triangle_intersection: Not exactly one of q0/t0 are set");
    }
    if ((c.l1 == nullptr) == (c.r1 == nullptr)) {
        THROW_OR_ABORT("handle_line_triangle_intersection: Not exactly one of l1/r1 are set");
    }
    const auto& L1 = (c.l1 != nullptr) ? c.l1->line : c.r1->edge;
    const auto& X1 = (c.l1 != nullptr) ? c.l1->ray : c.r1->ray;
#define l1 DO_NOT_USE_ME
#define r1 DO_NOT_USE_ME
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    ScenePos t;
    // if (!line_intersects_triangle(
    //     L1(0),
    //     L1(1),
    //     c.t0.triangle,
    //     t,
    //     &intersection_point))
    // {
    //     return;
    // }
    if (c.q0 != nullptr) {
        if (!X1.intersects(c.q0->polygon, &t, &intersection_point)) {
            return;
        }
    } else if (c.t0 != nullptr) {
        if (!X1.intersects(c.t0->polygon, &t, &intersection_point)) {
            return;
        }
    } else {
        verbose_abort("handle_line_triangle_intersection internal error");
    }
    if (any(c.mesh1_material & PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT) &&
        !c.l1_is_normal)
    {
        THROW_OR_ABORT("Unexpected c.l1_is_normal value");
    }
    c.o0.next_vehicle_domain_ = VehicleDomain::GROUND;
    c.o1.next_vehicle_domain_ = VehicleDomain::GROUND;
    if (c.l1_is_normal) {
        IntersectionSceneAndContact cc{
            .scene = c,
            .ray_t = t,
            .intersection_point = intersection_point};
        auto res = c.history.raycast_intersections.try_emplace(&L1, cc);
        if (!res.second) {
            if (cc.ray_t < res.first->second.ray_t) {
                c.history.raycast_intersections.erase(res.first);
                c.history.raycast_intersections.try_emplace(&L1, cc);
            }
        }
    } else if (any(c.mesh0_material & PhysicsMaterial::ATTR_CONCAVE) &&
               any(c.mesh1_material & PhysicsMaterial::ATTR_CONVEX))
    {
        c.history.concave_t0_intersections[&c.o1].push_back(
            IntersectionSceneAndContact{
                .scene = c,
                .ray_t = t,
                .intersection_point = intersection_point});
    } else {
        handle_line_triangle_intersection(c, intersection_point);
    }
}

void Mlib::handle_line_triangle_intersection(
    const IntersectionScene& c,
    const FixedArray<ScenePos, 3>& intersection_point)
{
#undef l1
#undef r1
    if ((c.q0 == nullptr) == (c.t0 == nullptr)) {
        THROW_OR_ABORT("handle_line_triangle_intersection: Not exactly one of q0/t0 are set");
    }
    if ((c.l1 == nullptr) == (c.r1 == nullptr)) {
        THROW_OR_ABORT("handle_line_triangle_intersection: Not exactly one of l1/e1 are set");
    }
    const auto& N0 = (c.t0 != nullptr) ? c.t0->polygon.plane() : c.q0->polygon.plane();
    const auto& X1 = (c.l1 != nullptr) ? c.l1->ray : c.r1->ray;
#define l1 DO_NOT_USE_ME
#define r1 DO_NOT_USE_ME
    CollisionType collision_type = c.default_collision_type;
    bool abort = false;
    for (auto& c0 : c.o0.collision_observers_) {
        c0->notify_collided(intersection_point, c.history.world, c.o1, CollisionRole::PRIMARY, collision_type, abort);
    }
    for (auto& c1 : c.o1.collision_observers_) {
        c1->notify_collided(intersection_point, c.history.world, c.o0, CollisionRole::SECONDARY, collision_type, abort);
    }
    c.history.csg.notify_contact(intersection_point, fixed_zeros<float, 3>(), N0.normal, c);
    if (abort) {
        return;
    }
    if (collision_type == CollisionType::GO_THROUGH) {
        // do nothing
    } else if (collision_type == CollisionType::REFLECT) {
        handle_reflection(
            c,
            intersection_point,
            c.surface_contact_info == nullptr
                ? 1.f
                : c.surface_contact_info->stiction_factor);
    } else if (collision_type == CollisionType::GRIND) {
        if (!c.o0.grind_state_.wants_to_grind_) {
            return;
        }
        FixedArray<float, 3> d3 = (intersection_point - c.o0.abs_grind_point()).casted<float>();
        if (std::abs(dot0d(X1.direction, N0.normal)) < c.history.cfg.max_grind_cos) {
            return;
        }
        bool direction_ok = false;
        if (!any(Mlib::isnan(c.o0.grind_state_.grind_direction_))) {
            float vl = std::abs(dot0d(c.o0.grind_state_.grind_direction_, X1.direction.casted<float>()));
            if (vl > c.history.cfg.continuos_grind_cos_threshold) {
                direction_ok = true;
            }
        }
        if (!direction_ok) {
            if (c.o0.grind_state_.wants_to_grind_counter_ > c.history.cfg.nframes_straight_grind) {
                float v_len2 = sum(squared(c.o0.rbp_.v_));
                if (v_len2 > squared(c.history.cfg.continuos_grind_velocity_threshold)) {
                    float vl = std::abs(dot0d(c.o0.rbp_.v_, X1.direction.casted<float>()) / std::sqrt(v_len2));
                    if (vl < c.history.cfg.continuos_grind_cos_threshold) {
                        return;
                    }
                }
            }
        }
        GrindInfo gi{
            .squared_distance = sum(squared(d3)),
            .intersection_point = intersection_point,
            .rail_direction = X1.direction,
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
