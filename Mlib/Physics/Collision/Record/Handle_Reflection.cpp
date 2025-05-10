#include "Handle_Reflection.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Intersection/Intersectors/Intersection_Info.hpp>
#include <Mlib/Geometry/Mesh/Farthest_Distances.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Physics/Actuators/Tire.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Compute_Edge_Overlap.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Collision/Resolve/Constraints.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Jump.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Attached_Wheel.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

using namespace Mlib;

static void handle_standard_reflection(
    const IntersectionScene& c,
    const FixedArray<SceneDir, 3>& normal,
    const FixedArray<ScenePos, 3>& intersection_point,
    float overlap)
{
    assert_true((c.o0.mass() != INFINITY) && (c.o1.mass() == INFINITY));
    assert_true(c.tire_id1 == SIZE_MAX);

    // Normal force
    auto ci = std::make_unique<NormalContactInfo1>(
        c.o0.rbp_,
        BoundedPlaneInequalityConstraint{
            .constraint{
                .normal_impulse{.normal = -normal},
                .overlap = overlap,
                .slop = 0.001f,
                .beta = c.history.cfg.plane_inequality_beta
            },
            .lambda_min = c.o0.mass() * c.history.cfg.velocity_lambda_min,
            .lambda_max = 0},
        intersection_point);
    const NormalImpulse* normal_impulse = &ci->normal_impulse();
    c.history.contact_infos.push_back(std::move(ci));

    // Tangential force
    c.history.contact_infos.push_back(std::unique_ptr<IContactInfo>(new FrictionContactInfo1{
        c.o0.rbp_,
        *normal_impulse,
        intersection_point,
        c.surface_contact_info != nullptr ? c.surface_contact_info->stiction_coefficient : c.history.cfg.stiction_coefficient,
        c.surface_contact_info != nullptr ? c.surface_contact_info->friction_coefficient : c.history.cfg.friction_coefficient,
        c.o1.velocity_at_position(intersection_point)}));
}

static void handle_extended_reflection(
    const IntersectionScene& c,
    const FixedArray<SceneDir, 3>& normal,
    const FixedArray<ScenePos, 3>& intersection_point,
    float overlap,
    float surface_stiction_factor)
{
    // ################
    // # Normal force #
    // ################
    const NormalImpulse* normal_impulse = nullptr;
    if (c.o0.mass() != INFINITY) {
        auto ci = std::make_unique<NormalContactInfo2>(
            c.o1.rbp_,
            c.o0.rbp_,
            BoundedPlaneInequalityConstraint{
                .constraint{
                    .normal_impulse{.normal = normal},
                    .overlap = overlap,
                    .slop = (c.tire_id1 != SIZE_MAX)  // tires have a shock-absorber, so disable slop
                        ? 0.001f
                        : 0.f,
                    .beta = c.history.cfg.plane_inequality_beta
                },
                .lambda_min = (c.o0.mass() * c.o1.mass()) / (c.o0.mass() + c.o1.mass()) * c.history.cfg.velocity_lambda_min,
                .lambda_max = 0},
            // intersection_point};
            c.tire_id1 != SIZE_MAX
                ? c.o1.get_abs_tire_contact_position(c.tire_id1)
                : intersection_point,
            [c, normal](float lambda_final){
                for (auto& c0 : c.o0.collision_observers_) {
                    c0->notify_impact(c.o1, CollisionRole::PRIMARY, normal.casted<float>(), lambda_final, c.history.base_log);
                }
                for (auto& c1 : c.o1.collision_observers_) {
                    c1->notify_impact(c.o0, CollisionRole::SECONDARY, normal.casted<float>(), lambda_final, c.history.base_log);
                }
            });
        normal_impulse = &ci->normal_impulse();
        c.history.contact_infos.push_back(std::move(ci));
    } else {
        if (c.tire_id1 == SIZE_MAX) {
            auto ci = std::make_unique<NormalContactInfo1>(
                c.o1.rbp_,
                BoundedPlaneInequalityConstraint{
                    .constraint{
                        .normal_impulse{.normal = normal},
                        .overlap = overlap,
                        .slop = 0.001f,
                        .beta = c.history.cfg.plane_inequality_beta
                    },
                    .lambda_min = c.o1.mass() * c.history.cfg.velocity_lambda_min,
                    .lambda_max = 0},
                intersection_point);
            normal_impulse = &ci->normal_impulse();
            c.history.contact_infos.push_back(std::move(ci));
        } else {
            if (c.o1.jump_state_.wants_to_jump_oversampled_ &&
                !c.o1.grind_state_.grinding_ &&
                !any(c.mesh0_material & PhysicsMaterial::OBJ_ALIGNMENT_PLANE))
            {
                jump(c.o0.rbp_, c.o1.rbp_, c.o1.jump_dv_, { .vector = normal.casted<float>(), .position = intersection_point });
            }
            auto& tire = c.o1.tires_.get(c.tire_id1);
            if (tire.rbp != nullptr) {
                float fsap = -(float)dot0d(tire.rbp->abs_position() - intersection_point, c.l1->ray.direction.casted<ScenePos>()) - tire.radius;
                if (fsap < 0.f) {
                    auto ci = std::make_unique<AttachedWheelNormalContactInfo1>(
                        AttachedWheel{ c.o1.rbp_, *tire.rbp, tire.vertical_line },
                        BoundedPlaneInequalityConstraint{
                            .constraint{
                                .normal_impulse{.normal = normal},
                                .overlap = -fsap,
                                .slop = 0.001f,
                                .beta = c.history.cfg.plane_inequality_beta
                            },
                            .lambda_min = c.o1.mass() * c.history.cfg.velocity_lambda_min,
                            .lambda_max = 0
                        },
                        intersection_point);
                    c.history.contact_infos.push_back(std::move(ci));
                }
                if (tire.normal_impulse == nullptr) {
                    THROW_OR_ABORT("Tire normal_impulse not set");
                }
                normal_impulse = tire.normal_impulse;
            } else {
                float fit = -dot0d(normal.casted<float>(), c.l1->ray.direction.casted<float>());
                if (fit < 1e-12) {
                    return;
                }
                float sap = std::min(0.05f, c.history.cfg.wheel_penetration_depth - overlap / fit);
                tire.shock_absorber_position = -sap;
                auto ci = std::make_unique<ShockAbsorberContactInfo1>(
                    c.o1.rbp_,
                    BoundedShockAbsorberConstraint{
                        .constraint{
                            .normal_impulse{.normal = normal},
                            .fit = fit,
                            .distance = sap,
                            .Ks = tire.sKs,
                            .Ka = tire.sKa
                        },
                        .lambda_min = c.o1.mass() * c.history.cfg.velocity_lambda_min,
                        .lambda_max = 0 },
                    intersection_point);
                normal_impulse = &ci->normal_impulse();
                c.history.contact_infos.push_back(std::move(ci));
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
    // ####################
    // # Tangential force #
    // ####################
    FixedArray<float, 3> tangential_force = uninitialized;
    bool align = any(c.mesh0_material & PhysicsMaterial::OBJ_ALIGNMENT_PLANE);
    if (c.o0.mass() == INFINITY && c.o1.mass() != INFINITY) {
        if (c.tire_id1 != SIZE_MAX) {
            FixedArray<float, 3> n3 = c.o1.get_abs_tire_z(c.tire_id1);
            n3 -= normal.casted<float>() * dot0d(normal.casted<float>(), n3);
            if (float len2 = sum(squared(n3)); len2 > 1e-12) {
                n3 /= std::sqrt(len2);
                if (normal_impulse != nullptr) {
                    FixedArray<float, 3> vc = c.o1.rbp_.v_com_;
                    vc -= normal.casted<float>() * dot0d(normal.casted<float>(), vc);
                    FixedArray<ScenePos, 3> contact_position = c.o1.get_abs_tire_contact_position(c.tire_id1);
                    FixedArray<float, 3> v_street = c.o0.velocity_at_position(contact_position);
                    FixedArray<float, 3> vc_street = c.o0.velocity_at_position(c.o1.abs_com());
                    c.history.contact_infos.push_back(std::unique_ptr<IContactInfo>(new TireContactInfo1{
                        FrictionContactInfo1{
                            c.o1.rbp_,
                            *normal_impulse,
                            contact_position,
                            NAN, // clamping handled by "TireContactInfo1" // c.o1.tires_.at(c.tire_id1).stiction_coefficient(-force_n1),
                            NAN, // clamping handled by "TireContactInfo1" // c.o1.tires_.at(c.tire_id1).friction_coefficient(-force_n1),
                            v_street},
                        surface_stiction_factor,
                        c.o1,
                        c.tire_id1,
                        vc_street,
                        vc,
                        n3,
                        -dot0d(c.o1.get_velocity_at_tire_contact(normal.casted<float>(), c.tire_id1) - v_street, n3),
                        c.history.cfg}));
                    // if (c.beacons != nullptr) {
                    //     c.beacons->push_back(Beacon::create(contact_position, "beacon"));
                    // }
                }
                // lerr() << "P " << P << " Pi " << power_internal << " Pe " << power_external << " " << (P > power_internal);
            } else {
                tangential_force = 0;
            }
        } else {
            c.history.contact_infos.push_back(std::unique_ptr<IContactInfo>(new FrictionContactInfo1{
                c.o1.rbp_,
                *normal_impulse,
                intersection_point,
                align ? 0.f : c.surface_contact_info != nullptr ? c.surface_contact_info->stiction_coefficient : c.history.cfg.stiction_coefficient,
                align ? 0.f : c.surface_contact_info != nullptr ? c.surface_contact_info->friction_coefficient : c.history.cfg.friction_coefficient,
                c.o0.velocity_at_position(intersection_point)}));
        }
    } else {
        c.history.contact_infos.push_back(std::unique_ptr<IContactInfo>(new FrictionContactInfo2{
            c.o1.rbp_,
            c.o0.rbp_,
            *normal_impulse,
            intersection_point,
            align ? 0.f : c.surface_contact_info != nullptr ? c.surface_contact_info->stiction_coefficient : c.history.cfg.stiction_coefficient,
            align ? 0.f : c.surface_contact_info != nullptr ? c.surface_contact_info->friction_coefficient : c.history.cfg.friction_coefficient,
            fixed_zeros<float, 3>()}));
    }
    // if (float lr = c.cfg.stiction_coefficient * force_n1; lr > 1e-12) {
    //     lerr() << "f " << c.tire_id1 << " " << std::sqrt(sum(squared(tangential_force))) / lr;
    // }
}

void Mlib::handle_reflection(
    const IntersectionScene& c,
    const IntersectionInfo& iinfo,
    float surface_stiction_factor)
{
    const auto* N0 = (c.t0.has_value()) ? &c.t0->polygon.plane : c.q0.has_value() ? &c.q0->polygon.plane : nullptr;

    // #############
    // # Alignment #
    // #############
    if (any(c.mesh0_material & PhysicsMaterial::OBJ_ALIGNMENT_PLANE)) {
        if (c.o1.grind_state_.wants_to_grind_)
        {
            // If "!wants_to_grind_", touches_alignment_plane_ means that
            // the currently stored surface normal was generated by
            // an alignent plane.
            c.o1.align_to_surface_state_.touches_alignment_plane_ = true;
            return;
        }
        if (N0 == nullptr) {
            THROW_OR_ABORT("Alignment object has no plane normal");
        }
        if ((dot0d(N0->normal.casted<float>(), c.o1.rbp_.rotation_.column(1)) < c.history.cfg.alignment_plane_cos) ||
            !std::isnan(c.o1.fly_forward_state_.wants_to_fly_forward_factor_))
        {
            return;
        }
    }
    if (any(c.mesh1_material & PhysicsMaterial::OBJ_ALIGNMENT_CONTACT)) {
        if (N0 == nullptr) {
            THROW_OR_ABORT("Alignment contact touches an object without a plane normal");
        }
        if (c.o1.align_to_surface_state_.align_to_surface_relaxation_ != 0.f) {
            if (any(c.mesh0_material & PhysicsMaterial::OBJ_ALIGNMENT_PLANE)) {
                if (!c.o1.align_to_surface_state_.touches_alignment_plane_ ||
                    (N0->normal(1) > c.o1.align_to_surface_state_.surface_normal_(1)))
                {
                    c.o1.align_to_surface_state_.touches_alignment_plane_ = true;
                    c.o1.align_to_surface_state_.surface_normal_ = N0->normal.casted<float>();
                }
            } else if (!c.o1.align_to_surface_state_.touches_alignment_plane_ &&
                (std::abs(N0->normal(1)) > c.history.cfg.alignment_surface_cos) &&
                (!any(c.mesh0_material & PhysicsMaterial::ATTR_ALIGN_STRICT) ||
                    (N0->normal(1) > c.history.cfg.alignment_surface_cos_strict)) &&
                (// (dot0d(plane.normal, c.o1.rbp_.rotation_.column(1)) > c.cfg.alignment_cos) &&
                (any(Mlib::isnan(c.o1.align_to_surface_state_.surface_normal_)) ||
                (N0->normal(1) > c.o1.align_to_surface_state_.surface_normal_(1)))))
                // (c.o1.wants_to_grind_ && (plane.normal(1) > c.o1.surface_normal_(1))) ||
                // (!c.o1.wants_to_grind_ && (dot0d(plane.normal - c.o1.surface_normal_, c.o1.rbp_.rotation_.column(1)) > 0.f))))
            {
                c.o1.align_to_surface_state_.surface_normal_ = N0->normal.casted<float>();
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
    FixedArray<SceneDir, 3> normal = uninitialized;
    ScenePos overlap = INFINITY;
    if (iinfo.no.has_value()) {
        sat_used = true;
        overlap = iinfo.no->overlap;
        normal = iinfo.no->normal;
    } else if (!c.l1_is_normal) {
        assert_true(c.r1.has_value());
        IntersectionScene cf{ c };
        std::optional<CollisionPolygonSphere<CompressedScenePos, 4>> q0f;
        std::optional<CollisionPolygonSphere<CompressedScenePos, 3>> t0f;
        if (any(c.mesh0_material & PhysicsMaterial::ATTR_TWO_SIDED)) {
            if (!any(c.mesh1_material & PhysicsMaterial::ATTR_CONVEX)) {
                THROW_OR_ABORT("Two-sided materials require a convex collision partner (case 0). Consider using collision-normals.");
            }
            if (N0 == nullptr) {
                THROW_OR_ABORT("Two-sided materials require a collision partner with an plane normal");
            }
            auto dist = get_farthest_distances(*c.mesh1, *N0);
            // (dist.min + dist.max) / 2. < 0  =>  dist.min < -dist.max
            if (dist.min < -dist.max) {
                if (c.q0.has_value()) {
                    q0f = -(*c.q0);
                    cf.q0 = *q0f;
                }
                if (c.t0.has_value()) {
                    t0f = -(*c.t0);
                    cf.t0 = *t0f;
                }
            }
        }
        if (!compute_edge_overlap(cf, iinfo.intersection_point, sat_used, overlap, normal)) {
            return;
        }
    } else {
        if (!c.l1.has_value()) {
            THROW_OR_ABORT("handle_reflection: l1 not set");
        }
        if (N0 == nullptr) {
            THROW_OR_ABORT("Lines require a collision partner with either normals or plane normals");
        }
        normal = N0->normal;
        overlap = -(dot0d(c.l1->line[1].casted<ScenePos>(), normal.casted<ScenePos>()) + (ScenePos)N0->intercept);
        if (any(c.mesh0_material & PhysicsMaterial::ATTR_TWO_SIDED)) {
            if (overlap < 0) {
                normal = -normal;
                overlap = -overlap;
            }
        } else if (overlap < 1e-6) {
            // Epsilon enables two overlapping one-sided planes with
            // opposing normals.
            return;
        }
    }
    // if (c.history.beacons != nullptr) {
    //     c.history.beacons->push_back(Beacon::create(intersection_point, "beacon"));
    // }
    if (overlap < -1e-3) {
        if (sat_used) {
            THROW_OR_ABORT(
                "Line and triangle do not overlap. "
                "Are the objects non-convex? Gap: " +
                std::to_string(-overlap));
        } else {
            THROW_OR_ABORT(
                "Line and triangle do not overlap. "
                "Gap: " +
                std::to_string(-overlap));
        }
    }
    if (!c.l1_is_normal) {
        if (any(c.mesh0_material & PhysicsMaterial::ATTR_ROUND) ||
            any(c.mesh1_material & PhysicsMaterial::ATTR_ROUND))
        {
            FixedArray<SceneDir, 3> round_normal = uninitialized;
            assert_true(c.r1.has_value());
            if (any(c.mesh0_material & PhysicsMaterial::ATTR_ROUND) &&
                any(c.mesh1_material & PhysicsMaterial::ATTR_ROUND))
            {
                if (N0 == nullptr) {
                    THROW_OR_ABORT("Round materials require a plane normal (0)");
                }
                round_normal = N0->normal - c.r1->normal;
                SceneDir nl2 = sum(squared(round_normal));
                if (nl2 < 1e-12) {
                    THROW_OR_ABORT("Normal is too small in collision of round objects (objects might be unseparated)");
                }
                round_normal /= std::sqrt(nl2);
            } else if (any(c.mesh0_material & PhysicsMaterial::ATTR_ROUND)) {
                if (N0 == nullptr) {
                    THROW_OR_ABORT("Round materials require a plane normal (1)");
                }
                round_normal = N0->normal;
            } else {
                round_normal = -c.r1->normal;
            }
            if (dot0d(round_normal, normal) < c.history.cfg.max_cos_round_normal) {
                normal = round_normal;
            }
        } else {
            auto dv = c.o0.velocity_at_position(iinfo.intersection_point) - c.o1.velocity_at_position(iinfo.intersection_point);
            float vn = dot0d(normal.casted<float>(), dv);
            // if (vn > c.history.cfg.min_skip_velocity) {
            //     float ds = vn * c.history.cfg.dt_substeps();
            //     if (overlap < ds * c.history.cfg.slide_factor)
            //        (overlap > ds * c.history.cfg.ignore_factor))
            //     {
            //         return;
            //     }
            // }
            float ds = vn * c.history.cfg.dt_substeps();
            if (overlap < ds * c.history.cfg.slide_factor) {
                return;
            }
        }
    }
    if ((c.o0.mass() != INFINITY) && (c.o1.mass() == INFINITY)) {
        assert_true(sat_used);
        assert_true(!c.l1_is_normal);
        assert_true(c.tire_id1 == SIZE_MAX);
        assert_true(any(c.mesh0_material & PhysicsMaterial::ATTR_CONVEX));
        handle_standard_reflection(
            c,
            normal,
            iinfo.intersection_point,
            (float)overlap);
    } else {
        handle_extended_reflection(
            c,
            normal,
            iinfo.intersection_point,
            (float)overlap,
            surface_stiction_factor);
    }
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
