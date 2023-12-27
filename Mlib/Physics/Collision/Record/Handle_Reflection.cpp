#include "Handle_Reflection.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Sat_Normals.hpp>
#include <Mlib/Geometry/Mesh/Sat_Overlap.hpp>
#include <Mlib/Geometry/Mesh/Sat_Overlap2.hpp>
#include <Mlib/Geometry/Mesh/Static_Transformed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Collision/Resolve/Constraints.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

using namespace Mlib;

static void handle_standard_reflection(
    const IntersectionScene& c,
    const FixedArray<double, 3>& normal,
    const FixedArray<double, 3>& intersection_point,
    float overlap)
{
    assert_true((c.o0.mass() != INFINITY) && (c.o1.mass() == INFINITY));
    assert_true(c.tire_id1 == SIZE_MAX);

    // Normal force
    auto ci = std::make_unique<NormalContactInfo1>(
        c.o0.rbi_.rbp_,
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
    c.history.contact_infos.push_back(std::unique_ptr<ContactInfo>(new FrictionContactInfo1{
        c.o0.rbi_.rbp_,
        *normal_impulse,
        intersection_point,
        c.history.cfg.stiction_coefficient,
        c.history.cfg.friction_coefficient,
        c.o1.velocity_at_position(intersection_point)}));
}

static void handle_extended_reflection(
    const IntersectionScene& c,
    const FixedArray<double, 3>& normal,
    const FixedArray<double, 3>& intersection_point,
    const FixedArray<double, 3>* penetrating_point,
    float overlap,
    float surface_stiction_factor)
{
    // ################
    // # Normal force #
    // ################
    const NormalImpulse* normal_impulse = nullptr;
    if (c.o0.mass() != INFINITY) {
        auto ci = std::make_unique<NormalContactInfo2>(
            c.o1.rbi_.rbp_,
            c.o0.rbi_.rbp_,
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
                c.o1.rbi_.rbp_,
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
            if (penetrating_point == nullptr) {
                THROW_OR_ABORT("Penetrating point not set");
            }
            float penetration_depth = (float)dot0d(*penetrating_point - intersection_point, normal);
            if (c.o1.jump_state_.wants_to_jump_oversampled_ &&
                !c.o1.grind_state_.grinding_ &&
                !any(c.mesh0_material & PhysicsMaterial::OBJ_ALIGNMENT_PLANE))
            {
                penetration_depth -= c.o1.jump_strength_ * (float)c.history.cfg.oversampling;
            }
            float sap = std::min(0.05f, c.history.cfg.wheel_penetration_depth + penetration_depth);
            c.o1.tires_.at(c.tire_id1).shock_absorber_position = -sap;
            auto ci = std::make_unique<ShockAbsorberContactInfo1>(
                c.o1.rbi_.rbp_,
                BoundedShockAbsorberConstraint{
                    .constraint{
                        .normal_impulse{.normal = normal},
                        .distance = sap,
                        .Ks = c.o1.tires_.at(c.tire_id1).sKs,
                        .Ka = c.o1.tires_.at(c.tire_id1).sKa
                    },
                    .lambda_min = c.o1.mass() * c.history.cfg.velocity_lambda_min,
                    .lambda_max = 0},
                intersection_point);
            normal_impulse = &ci->normal_impulse();
            c.history.contact_infos.push_back(std::move(ci));
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
    FixedArray<float, 3> tangential_force;
    bool align = any(c.mesh0_material & PhysicsMaterial::OBJ_ALIGNMENT_PLANE);
    if (c.o0.mass() == INFINITY && c.o1.mass() != INFINITY) {
        if (c.tire_id1 != SIZE_MAX) {
            FixedArray<float, 3> n3 = c.o1.get_abs_tire_z(c.tire_id1);
            n3 -= normal.casted<float>() * dot0d(normal.casted<float>(), n3);
            if (float len2 = sum(squared(n3)); len2 > 1e-12) {
                n3 /= std::sqrt(len2);
                if (normal_impulse != nullptr) {
                    FixedArray<float, 3> vc = c.o1.rbi_.rbp_.v_;
                    vc -= normal.casted<float>() * dot0d(normal.casted<float>(), vc);
                    FixedArray<double, 3> contact_position = c.o1.get_abs_tire_contact_position(c.tire_id1);
                    FixedArray<float, 3> v_street = c.o0.velocity_at_position(contact_position);
                    FixedArray<float, 3> vc_street = c.o0.velocity_at_position(c.o1.abs_com());
                    c.history.contact_infos.push_back(std::unique_ptr<ContactInfo>(new TireContactInfo1{
                        FrictionContactInfo1{
                            c.o1.rbi_.rbp_,
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
                // std::cerr << "P " << P << " Pi " << power_internal << " Pe " << power_external << " " << (P > power_internal) << std::endl;
            } else {
                tangential_force = 0;
            }
        } else {
            c.history.contact_infos.push_back(std::unique_ptr<ContactInfo>(new FrictionContactInfo1{
                c.o1.rbi_.rbp_,
                *normal_impulse,
                intersection_point,
                align ? 0.f : c.history.cfg.stiction_coefficient,
                align ? 0.f : c.history.cfg.friction_coefficient,
                c.o0.velocity_at_position(intersection_point)}));
        }
    } else {
        c.history.contact_infos.push_back(std::unique_ptr<ContactInfo>(new FrictionContactInfo2{
            c.o1.rbi_.rbp_,
            c.o0.rbi_.rbp_,
            *normal_impulse,
            intersection_point,
            align ? 0.f : c.history.cfg.stiction_coefficient,
            align ? 0.f : c.history.cfg.friction_coefficient,
            fixed_zeros<float, 3>()}));
    }
    // if (float lr = c.cfg.stiction_coefficient * force_n1; lr > 1e-12) {
    //     std::cerr << "f " << c.tire_id1 << " " << std::sqrt(sum(squared(tangential_force))) / lr << std::endl;
    // }
}

void Mlib::handle_reflection(
    const IntersectionScene& c,
    const FixedArray<double, 3>& intersection_point,
    float surface_stiction_factor)
{
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
        if ((dot0d(c.t0.plane.normal.casted<float>(), c.o1.rbi_.rbp_.rotation_.column(1)) < c.history.cfg.alignment_plane_cos) ||
            !std::isnan(c.o1.fly_forward_state_.wants_to_fly_forward_factor_))
        {
            return;
        }
    }
    if (any(c.mesh1_material & PhysicsMaterial::OBJ_ALIGNMENT_CONTACT)) {
        if (c.o1.align_to_surface_state_.align_to_surface_relaxation_ != 0.f) {
            if (any(c.mesh0_material & PhysicsMaterial::OBJ_ALIGNMENT_PLANE)) {
                if (!c.o1.align_to_surface_state_.touches_alignment_plane_ ||
                    (c.t0.plane.normal(1) > c.o1.align_to_surface_state_.surface_normal_(1)))
                {
                    c.o1.align_to_surface_state_.touches_alignment_plane_ = true;
                    c.o1.align_to_surface_state_.surface_normal_ = c.t0.plane.normal.casted<float>();
                }
            } else if (!c.o1.align_to_surface_state_.touches_alignment_plane_ &&
                (std::abs(c.t0.plane.normal(1)) > c.history.cfg.alignment_surface_cos) &&
                (!any(c.mesh0_material & PhysicsMaterial::ATTR_ALIGN_STRICT) ||
                    (c.t0.plane.normal(1) > c.history.cfg.alignment_surface_cos_strict)) &&
                (// (dot0d(plane.normal, c.o1.rbi_.rbp_.rotation_.column(1)) > c.cfg.alignment_cos) &&
                (any(Mlib::isnan(c.o1.align_to_surface_state_.surface_normal_)) ||
                (c.t0.plane.normal(1) > c.o1.align_to_surface_state_.surface_normal_(1)))))
                // (c.o1.wants_to_grind_ && (plane.normal(1) > c.o1.surface_normal_(1))) ||
                // (!c.o1.wants_to_grind_ && (dot0d(plane.normal - c.o1.surface_normal_, c.o1.rbi_.rbp_.rotation_.column(1)) > 0.f))))
            {
                c.o1.align_to_surface_state_.surface_normal_ = c.t0.plane.normal.casted<float>();
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
    FixedArray<double, 3> normal;
    double overlap = INFINITY;
    const FixedArray<double, 3>* penetrating_point = nullptr;
    if (!c.l1_is_normal &&
        any(c.mesh0_material & PhysicsMaterial::ATTR_CONVEX) &&
        any(c.mesh1_material & PhysicsMaterial::ATTR_CONVEX))
    {
        sat_used = true;
        assert_true(c.mesh0 != nullptr);
        assert_true(c.mesh1 != nullptr);
        try {
            c.history.st.get_collision_plane(*c.mesh0, *c.mesh1, overlap, normal);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(
                "Could not compute collision plane of meshes \"" + c.mesh0->name() + "\" and \"" + c.mesh1->name() + "\": " + e.what());
        }
    } else if (!c.l1_is_normal &&
               any(c.mesh0_material & PhysicsMaterial::ATTR_CONCAVE) &&
               any(c.mesh1_material & PhysicsMaterial::ATTR_CONVEX))
    {
        if (dot0d(intersection_point - c.o1.rbi_.rbp_.abs_position(), c.t0.plane.normal) > 0.) {
            return;
        }
        sat_used = true;
        // assert_true(c.mesh1 != nullptr);
        // overlap = get_overlap(c.t0, *c.mesh1);
        // if (overlap > 0.5) {
        //     return;
        // }

        std::vector<CollisionRidgeSphere> ridges;
        ridges.reserve(3);
        for (size_t i = 0; i < 3; ++i) {
            auto a = OrderableFixedArray{c.t0.triangle(i)};
            auto b = OrderableFixedArray{c.t0.triangle((i + 1) % 3)};
            auto it = (a < b)
                ? c.history.ridge_map.find({a, b})
                : c.history.ridge_map.find({b, a});
            if (it == c.history.ridge_map.end()) {
                // Ridges that cannot be collided due to their angle are removed,
                // so failure is expected.
                continue;
            }
            ridges.push_back(*it->second);
        }
        StaticTransformedMesh stm(
            "temp",
            AxisAlignedBoundingBox<double, 3>{c.t0.triangle},
            BoundingSphere<double, 3>{c.t0.triangle},
            std::vector<CollisionTriangleSphere>{c.t0},
            std::vector<CollisionLineSphere>(),
            std::vector<CollisionLineSphere>(),
            std::move(ridges));

        assert_true(c.r1 != nullptr);
        try {
            get_overlap2(stm, *c.r1, -INFINITY, overlap, normal);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(
                "Could not compute collision plane of temporary mesh and edge: " + std::string(e.what()));
        }
        if (overlap == INFINITY) {
            return;
        }
        if (dot0d(c.t0.plane.normal, normal) < c.history.cfg.min_cos_ridge_triangle) {
            return;
        }
        if (dot0d(intersection_point - c.o1.rbi_.rbp_.abs_position(), normal) > 0.) {
            return;
        }
    } else if (!c.l1_is_normal &&
               any(c.mesh0_material & PhysicsMaterial::ATTR_CONVEX) &&
               any(c.mesh1_material & PhysicsMaterial::ATTR_CONCAVE))
    {
        sat_used = true;
        assert_true(c.mesh0 != nullptr);
        assert_true(c.r1 != nullptr);
        try {
            get_overlap2(*c.mesh0, *c.r1, c.history.cfg.max_keep_normal, overlap, normal);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(
                "Could not compute collision plane of mesh \"" + c.mesh0->name() + "\" and edge: " + e.what());
        }
        if (overlap == INFINITY) {
            return;
        }
        if (dot0d(intersection_point - c.o0.rbi_.rbp_.abs_position(), normal) < 0.) {
            return;
        }
        // if (overlap > (double)c.history.cfg.overlap_ignored) {
        //     return;
        // }
        // overlap = std::min((double)c.history.cfg.overlap_clipped, overlap);
        c.history.ridge_intersection_points[&c.o0].push_back(intersection_point);
    } else if (!c.l1_is_normal) {
        bool first_convex = any(c.mesh0_material & PhysicsMaterial::ATTR_CONVEX);
        bool second_convex = any(c.mesh1_material & PhysicsMaterial::ATTR_CONVEX);
        THROW_OR_ABORT(
            "Physics material of all objects is not convex (object \"" +
            c.o0.name() + "\", mesh \"" +
            (c.mesh0 == nullptr ? "<null>" : c.mesh0->name()) +
            "\", object \"" +
            c.o1.name() + "\", mesh \"" +
            (c.mesh1 == nullptr ? "<null>" : c.mesh1->name()) + "\"), convexity: " +
            std::to_string(int(first_convex)) + ", " + std::to_string(int(second_convex)));
    } else {
        assert_true(c.l1_is_normal);
        if (c.l1 == nullptr) {
            THROW_OR_ABORT("handle_reflection: l1 not set");
        }
        normal = c.t0.plane.normal;
        overlap = -(dot0d(c.l1->line(1), normal) + c.t0.plane.intercept);
        if (any(c.mesh0_material & PhysicsMaterial::ATTR_TWO_SIDED)) {
            if (overlap < 0) {
                normal *= -1;
                overlap *= -1;
            }
        } else if (overlap < 1e-6) {
            // Epsilon enables two overlapping one-sided planes with
            // opposing normals.
            return;
        }
        penetrating_point = &c.l1->line(1);
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
            FixedArray<double, 3> round_normal;
            assert_true(c.r1 != nullptr);
            if (any(c.mesh0_material & PhysicsMaterial::ATTR_ROUND) &&
                any(c.mesh1_material & PhysicsMaterial::ATTR_ROUND))
            {
                round_normal = c.t0.plane.normal - c.r1->normal;
                double nl2 = sum(squared(round_normal));
                if (nl2 < 1e-12) {
                    THROW_OR_ABORT("Normal is too small in collision of round objects (objects might be unseparated)");
                }
                round_normal /= std::sqrt(nl2);
            } else if (any(c.mesh0_material & PhysicsMaterial::ATTR_ROUND)) {
                round_normal = c.t0.plane.normal;
            } else {
                round_normal = -c.r1->normal;
            }
            if (dot0d(round_normal, normal) < c.history.cfg.max_cos_round_normal) {
                normal = round_normal;
            }
        } else {
            auto dv = c.o0.velocity_at_position(intersection_point) - c.o1.velocity_at_position(intersection_point);
            float vn = dot0d(normal.casted<float>(), dv);
            if (overlap < vn * c.history.cfg.slide_factor) {
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
            intersection_point,
            (float)overlap);
    } else {
        handle_extended_reflection(
            c,
            normal,
            intersection_point,
            penetrating_point,
            (float)overlap,
            surface_stiction_factor);
    }
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
