#include "Handle_Reflection.hpp"
#include <Mlib/Geometry/Mesh/Intersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Sat_Normals.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Collision/Resolve/Constraints.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

using namespace Mlib;

static void handle_standard_reflection(
    const IntersectionScene& c,
    const PlaneNd<double, 3>& plane,
    const FixedArray<double, 3>& intersection_point,
    const FixedArray<double, 3>& penetrating_point)
{
    assert_true((c.o0.mass() != INFINITY) && (c.o1.mass() == INFINITY));
    assert_true(c.tire_id1 == SIZE_MAX);

    // Normal force
    auto ci = std::make_unique<NormalContactInfo1>(
        c.o0.rbi_.rbp_,
        BoundedPlaneInequalityConstraint{
            .constraint{
                .normal_impulse{.normal = -plane.normal},
                .intercept = -plane.intercept,
                .slop = 0.001f,
                .beta = c.history.cfg.plane_inequality_beta
            },
            .lambda_min = c.o0.mass() * c.history.cfg.velocity_lambda_min,
            .lambda_max = 0},
        penetrating_point);
    const NormalImpulse* normal_impulse = &ci->normal_impulse();
    c.history.contact_infos.push_back(std::move(ci));

    // Tangential force
    c.history.contact_infos.push_back(std::unique_ptr<ContactInfo>(new FrictionContactInfo1{
        c.o0.rbi_.rbp_,
        *normal_impulse,
        penetrating_point,
        c.history.cfg.stiction_coefficient,
        c.history.cfg.friction_coefficient,
        c.o1.velocity_at_position(penetrating_point)}));
}

static void handle_extended_reflection(
    const IntersectionScene& c,
    const PlaneNd<double, 3>& plane,
    const FixedArray<double, 3>& intersection_point,
    const FixedArray<double, 3>& penetrating_point)
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
                    .normal_impulse{.normal = plane.normal},
                    .intercept = plane.intercept,
                    .slop = (c.tire_id1 != SIZE_MAX)
                        ? 0.001f
                        : 0.f,
                    .beta = c.history.cfg.plane_inequality_beta
                },
                .lambda_min = (c.o0.mass() * c.o1.mass()) / (c.o0.mass() + c.o1.mass()) * c.history.cfg.velocity_lambda_min,
                .lambda_max = 0},
            // penetrating_point};
            c.tire_id1 != SIZE_MAX
                ? c.o1.get_abs_tire_contact_position(c.tire_id1)
                : penetrating_point,
            [c, plane](float lambda_final){
                for (auto& c0 : c.o0.collision_observers_) {
                    c0->notify_impact(c.o1, CollisionRole::PRIMARY, plane.normal.casted<float>(), lambda_final, c.history.base_log);
                }
                for (auto& c1 : c.o1.collision_observers_) {
                    c1->notify_impact(c.o0, CollisionRole::SECONDARY, plane.normal.casted<float>(), lambda_final, c.history.base_log);
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
                        .normal_impulse{.normal = plane.normal},
                        .intercept = plane.intercept,
                        .slop = 0.001f,
                        .beta = c.history.cfg.plane_inequality_beta
                    },
                    .lambda_min = c.o1.mass() * c.history.cfg.velocity_lambda_min,
                    .lambda_max = 0},
                penetrating_point);
            normal_impulse = &ci->normal_impulse();
            c.history.contact_infos.push_back(std::move(ci));
        } else {
            float penetration_depth = dot0d(penetrating_point - intersection_point, plane.normal);
            if (c.o1.jump_state_.wants_to_jump_oversampled_ &&
                !c.o1.grind_state_.grinding_ &&
                !any(c.mesh0_material & PhysicsMaterial::OBJ_ALIGNMENT_PLANE))
            {
                penetration_depth -= c.o1.jump_strength_ * c.history.cfg.oversampling;
            }
            float sap = std::min(0.05f, c.history.cfg.wheel_penetration_depth + penetration_depth);
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
            n3 -= plane.normal.casted<float>() * dot0d(plane.normal.casted<float>(), n3);
            if (float len2 = sum(squared(n3)); len2 > 1e-12) {
                n3 /= std::sqrt(len2);
                if (normal_impulse != nullptr) {
                    FixedArray<float, 3> vc = c.o1.rbi_.rbp_.v_;
                    vc -= plane.normal.casted<float>() * dot0d(plane.normal.casted<float>(), vc);
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
                        c.o1,
                        c.tire_id1,
                        vc_street,
                        vc,
                        n3,
                        -dot0d(c.o1.get_velocity_at_tire_contact(plane.normal.casted<float>(), c.tire_id1) - v_street, n3),
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
                penetrating_point,
                align ? 0.f : c.history.cfg.stiction_coefficient,
                align ? 0.f : c.history.cfg.friction_coefficient,
                c.o0.velocity_at_position(penetrating_point)}));
        }
    } else {
        c.history.contact_infos.push_back(std::unique_ptr<ContactInfo>(new FrictionContactInfo2{
            c.o1.rbi_.rbp_,
            c.o0.rbi_.rbp_,
            *normal_impulse,
            penetrating_point,
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
    const FixedArray<double, 3>& intersection_point)
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
        if ((dot0d(c.p0.normal.casted<float>(), c.o1.rbi_.rbp_.rotation_.column(1)) < c.history.cfg.alignment_plane_cos) ||
            !std::isnan(c.o1.fly_forward_state_.wants_to_fly_forward_factor_))
        {
            return;
        }
    }
    if (any(c.mesh1_material & PhysicsMaterial::OBJ_ALIGNMENT_CONTACT)) {
        if (c.o1.align_to_surface_state_.align_to_surface_relaxation_ != 0.f) {
            if (any(c.mesh0_material & PhysicsMaterial::OBJ_ALIGNMENT_PLANE)) {
                if (!c.o1.align_to_surface_state_.touches_alignment_plane_ ||
                    (c.p0.normal(1) > c.o1.align_to_surface_state_.surface_normal_(1)))
                {
                    c.o1.align_to_surface_state_.touches_alignment_plane_ = true;
                    c.o1.align_to_surface_state_.surface_normal_ = c.p0.normal.casted<float>();
                }
            } else if (!c.o1.align_to_surface_state_.touches_alignment_plane_ &&
                (std::abs(c.p0.normal(1)) > c.history.cfg.alignment_surface_cos) &&
                (!any(c.mesh0_material & PhysicsMaterial::ATTR_ALIGN_STRICT) ||
                    (c.p0.normal(1) > c.history.cfg.alignment_surface_cos_strict)) &&
                (// (dot0d(plane.normal, c.o1.rbi_.rbp_.rotation_.column(1)) > c.cfg.alignment_cos) &&
                (any(Mlib::isnan(c.o1.align_to_surface_state_.surface_normal_)) ||
                (c.p0.normal(1) > c.o1.align_to_surface_state_.surface_normal_(1)))))
                // (c.o1.wants_to_grind_ && (plane.normal(1) > c.o1.surface_normal_(1))) ||
                // (!c.o1.wants_to_grind_ && (dot0d(plane.normal - c.o1.surface_normal_, c.o1.rbi_.rbp_.rotation_.column(1)) > 0.f))))
            {
                c.o1.align_to_surface_state_.surface_normal_ = c.p0.normal.casted<float>();
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
    PlaneNd<double, 3> plane;
    if (!c.l1_is_normal) {
        if (any(c.mesh0_material & PhysicsMaterial::ATTR_CONVEX) ==
            any(c.mesh0_material & PhysicsMaterial::ATTR_CONCAVE))
        {
            throw std::runtime_error(
                "Physics material is not convex xor concave (object \"" +
                c.o0.name() + "\", mesh \"" +
                (c.mesh0 == nullptr ? "<null>" : c.mesh0->name()) + "\")");
        }
        if (any(c.mesh1_material & PhysicsMaterial::ATTR_CONVEX) ==
            any(c.mesh1_material & PhysicsMaterial::ATTR_CONCAVE))
        {
            throw std::runtime_error(
                "Physics material is not convex xor concave (object \"" +
                c.o1.name() + "\", mesh \"" +
                (c.mesh1 == nullptr ? "<null>" : c.mesh1->name()) + "\")");
        }
    }
    if (!c.l1_is_normal &&
        any(c.mesh0_material & PhysicsMaterial::ATTR_CONVEX) &&
        any(c.mesh1_material & PhysicsMaterial::ATTR_CONVEX))
    {
        if (!c.history.cfg.sat) {
            plane = PlaneNd{
                c.o1.abs_com() - c.o0.abs_com(),
                intersection_point};
        } else {
            sat_used = true;
            // n = -st.get_collision_normal(o1, o0);
            // n = st->get_collision_normal(o0, o1);
            double min_overlap0;
            PlaneNd<double, 3> plane0;
            double min_overlap1;
            PlaneNd<double, 3> plane1;
            assert_true(c.mesh0 != nullptr);
            assert_true(c.mesh1 != nullptr);
            try {
                c.history.st.get_collision_plane(c.mesh0->get_triangles_sphere(), c.mesh1->get_triangles_sphere(), min_overlap0, plane0);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error(
                    "Could not compute collision plane of meshes \"" + c.mesh0->name() + "\" and \"" + c.mesh1->name() + "\": " + e.what());
            }
            try {
                c.history.st.get_collision_plane(c.mesh1->get_triangles_sphere(), c.mesh0->get_triangles_sphere(), min_overlap1, plane1);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error(
                    "Could not compute collision plane of meshes \"" + c.mesh1->name() + "\" and \"" + c.mesh0->name() + "\": " + e.what());
            }
            if (min_overlap0 < 0) {
                throw std::runtime_error("No overlap detected (0)");
            }
            if (min_overlap1 < 0) {
                throw std::runtime_error("No overlap detected (1)");
            }
            if (min_overlap0 > c.history.cfg.overlap_tolerance * min_overlap1) {
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
        if (any(c.mesh0_material & PhysicsMaterial::ATTR_TWO_SIDED)) {
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
        if (any(c.mesh0_material & PhysicsMaterial::ATTR_TWO_SIDED) &&
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
    //     c.beacons->push_back(Beacon::create(penetrating_point, "beacon"));
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
        dist = std::max(0.f, dist - c.history.cfg.wheel_penetration_depth);
    } else {
        dist = std::max(0.f, dist);
    }
    if ((c.o0.mass() != INFINITY) && (c.o1.mass() == INFINITY)) {
        assert_true(sat_used);
        assert_true(!c.l1_is_normal);
        assert_true(c.tire_id1 == SIZE_MAX);
        assert_true(any(c.mesh0_material & PhysicsMaterial::ATTR_CONVEX) &&
                    any(c.mesh1_material & PhysicsMaterial::ATTR_CONVEX));
        handle_standard_reflection(
            c,
            plane,
            intersection_point,
            c.l1(penetrating_id));
    } else {
        handle_extended_reflection(
            c,
            plane,
            intersection_point,
            c.l1(penetrating_id));
    }
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
