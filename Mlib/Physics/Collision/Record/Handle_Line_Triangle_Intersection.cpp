#include "Handle_Line_Triangle_Intersection.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Intersection/Intersectors/Intersection_Info.hpp>
#include <Mlib/Geometry/Intersection/Intersectors/Polygon_Line_Intersector.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Grind_Info.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Handle_Reflection.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Containers/Collision_Group.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Phase.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Domain.hpp>
#include <Mlib/Physics/Smoke_Generation/Contact_Smoke_Generator.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <format>

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
    if (int(c.q0.has_value()) + int(c.t0.has_value()) + int(c.i0 != nullptr) != 1) {
        THROW_OR_ABORT("handle_line_triangle_intersection: Not exactly one of q0/t0/i0 are set");
    }
    if (int(c.l1.has_value()) + int(c.r1.has_value()) + int(c.i1 != nullptr) != 1) {
        THROW_OR_ABORT("handle_line_triangle_intersection: Not exactly one of l1/r1/i1 are set");
    }
    if (((c.history.phase.group.penetration_class == PenetrationClass::BULLET_LINE) ==
        any(c.mesh0_material & PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT)) &&
        (c.o0.mass() != INFINITY) &&
        !c.history.phase.group.rigid_bodies.contains(&c.o0.rbp_))
    {
        THROW_OR_ABORT("Non-static rigid body \"" + c.o0.name() + "\" is not in the collision group (0)");
    }
    if (((c.history.phase.group.penetration_class == PenetrationClass::BULLET_LINE) ==
        any(c.mesh1_material & PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT)) &&
        (c.o1.mass() != INFINITY) &&
        !c.history.phase.group.rigid_bodies.contains(&c.o1.rbp_))
    {
        THROW_OR_ABORT("Non-static rigid body \"" + c.o1.name() + "\" is not in the collision group (1)");
    }
    if (c.o0.is_deactivated_avatar()) {
        THROW_OR_ABORT("Attempt to collide deactivated avatar (0): \"" + c.o0.name() + '"');
    }
    if (c.o1.is_deactivated_avatar()) {
        THROW_OR_ABORT("Attempt to collide deactivated avatar (1): \"" + c.o1.name() + '"');
    }
    if ((c.i0 != nullptr) && any(c.mesh1_material & PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT)) {
        THROW_OR_ABORT("Intersectable \"" + c.o0.name() + "\" unexpectedly collides with bullet line segment \"" + c.o1.name() + '"');
    }
    IntersectionInfo iinfo;
    try {
        if (c.q0.has_value()) {
            if (c.l1.has_value()) {
                if (!intersect(*c.q0, *c.l1, iinfo)) {
                    return;
                }
            } else if (c.r1.has_value()) {
                if (!intersect(*c.q0, *c.r1, iinfo)) {
                    return;
                }
            } else if (c.i1 != nullptr) {
                if (!intersect(*c.q0, *c.i1, iinfo)) {
                    return;
                }
            } else {
                THROW_OR_ABORT("Unexpected intersection object (0)");
            }
        } else if (c.t0.has_value()) {
            if (c.l1.has_value()) {
                if (!intersect(*c.t0, *c.l1, iinfo)) {
                    return;
                }
            } else if (c.r1.has_value()) {
                if (!intersect(*c.t0, *c.r1, iinfo)) {
                    return;
                }
            } else if (c.i1 != nullptr) {
                if (!intersect(*c.t0, *c.i1, iinfo)) {
                    return;
                }
            } else {
                THROW_OR_ABORT("Unexpected intersection object (1)");
            }
        } else if (c.i0 != nullptr) {
            if (c.l1.has_value()) {
                if (!intersect(*c.i0, *c.l1, iinfo)) {
                    return;
                }
            } else if (c.r1.has_value()) {
                if (!intersect(*c.i0, *c.r1, iinfo)) {
                    return;
                }
            } else if (c.i1 != nullptr) {
                if (!intersect(*c.i0, *c.i1, iinfo)) {
                    return;
                }
            } else {
                THROW_OR_ABORT("Unexpected intersection object (2)");
            }
        } else {
            THROW_OR_ABORT("Unexpected intersection object (3)");
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::format(
            "Error colliding objects \"{}\" and \"{}\", meshes \"{}\" and \"{}\": {}",
            c.o0.name(),
            c.o1.name(),
            (c.mesh0 == nullptr) ? "<null>" : c.mesh0->name(),
            (c.mesh1 == nullptr) ? "<null>" : c.mesh1->name(),
            e.what()));
    }
    // if (iinfo->has_normal_and_overlap()) {
    //     for (double t = 0; t < 0.5; t += 0.1) {
    //         add_beacon(Beacon::create(iinfo->intersection_point() + t * iinfo->normal(), "beacon"));
    //     }
    // }
    // if (!line_intersects_triangle(
    //     L1(0),
    //     L1(1),
    //     c.t0.triangle,
    //     t,
    //     &intersection_point))
    // {
    //     return;
    // }
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
            .iinfo = std::move(iinfo)};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
        assert_true(c.l1.has_value());
#pragma GCC diagnostic pop
        auto res = c.history.raycast_intersections.try_emplace(make_orderable(c.l1->line), std::move(cc));
        if (!res.second) {
            if (!cc.iinfo.ray_t.has_value()) {
                THROW_OR_ABORT("l1_is_normal but ray_t not given");
            }
            if (*cc.iinfo.ray_t < *res.first->second.iinfo.ray_t) {
                c.history.raycast_intersections.erase(res.first);
                c.history.raycast_intersections.try_emplace(make_orderable(c.l1->line), std::move(cc));
            }
        }
    } else if (any(c.mesh0_material & PhysicsMaterial::ATTR_CONCAVE) &&
               any(c.mesh1_material & PhysicsMaterial::ATTR_CONVEX))
    {
        c.history.concave_t0_intersections[&c.o1].push_back(
            IntersectionSceneAndContact{
                .scene = c,
                .iinfo = std::move(iinfo)});
    } else {
        handle_line_triangle_intersection(c, iinfo);
    }
}

void Mlib::handle_line_triangle_intersection(
    const IntersectionScene& c,
    const IntersectionInfo& iinfo)
{
    const auto* N0 = (c.t0.has_value()) ? &c.t0->polygon.plane : (c.q0.has_value()) ? &c.q0->polygon.plane : nullptr;
    const auto* X1 = (c.l1.has_value()) ? &c.l1->ray : (c.r1.has_value()) ? &c.r1->ray : nullptr;

    CollisionType collision_type = c.default_collision_type;
    bool abort = false;
    for (auto& c0 : c.o0.collision_observers_) {
        c0->notify_collided(iinfo.intersection_point, c.history.world, c.o1, CollisionRole::PRIMARY, collision_type, abort);
    }
    for (auto& c1 : c.o1.collision_observers_) {
        c1->notify_collided(iinfo.intersection_point, c.history.world, c.o0, CollisionRole::SECONDARY, collision_type, abort);
    }
    c.history.csg.notify_contact(iinfo.intersection_point, fixed_zeros<float, 3>(), iinfo.normal0, c);
    if (abort) {
        return;
    }
    if (collision_type == CollisionType::GO_THROUGH) {
        // do nothing
    } else if (collision_type == CollisionType::REFLECT) {
        handle_reflection(
            c,
            iinfo,
            c.surface_contact_info == nullptr
                ? 1.f
                : c.surface_contact_info->stiction_factor);
    } else if (collision_type == CollisionType::GRIND) {
        if (!c.o0.grind_state_.wants_to_grind_) {
            return;
        }
        if (N0 == nullptr) {
            THROW_OR_ABORT("Grind collision requires a plane normal");
        }
        if (X1 == nullptr) {
            THROW_OR_ABORT("Grind collision requires a ray");
        }
        FixedArray<float, 3> d3 = (iinfo.intersection_point - c.o0.abs_grind_point()).casted<float>();
        if (std::abs(dot0d(X1->direction, N0->normal)) < c.history.cfg.max_grind_cos) {
            return;
        }
        bool direction_ok = false;
        if (!any(Mlib::isnan(c.o0.grind_state_.grind_direction_))) {
            float vl = std::abs(dot0d(c.o0.grind_state_.grind_direction_, X1->direction.casted<float>()));
            if (vl > c.history.cfg.continuos_grind_cos_threshold) {
                direction_ok = true;
            }
        }
        if (!direction_ok) {
            if (c.o0.grind_state_.wants_to_grind_counter_ > c.history.cfg.nframes_straight_grind) {
                float v_len2 = sum(squared(c.o0.rbp_.v_com_));
                if (v_len2 > squared(c.history.cfg.continuos_grind_velocity_threshold)) {
                    float vl = std::abs(dot0d(c.o0.rbp_.v_com_, X1->direction.casted<float>()) / std::sqrt(v_len2));
                    if (vl < c.history.cfg.continuos_grind_cos_threshold) {
                        return;
                    }
                }
            }
        }
        GrindInfo gi{
            .squared_distance = sum(squared(d3)),
            .intersection_point = iinfo.intersection_point,
            .rail_direction = X1->direction,
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
