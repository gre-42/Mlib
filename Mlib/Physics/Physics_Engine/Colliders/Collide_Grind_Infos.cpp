#include "Collide_Grind_Infos.hpp"
#include <Mlib/Physics/Collision/Grind_Info.hpp>
#include <Mlib/Physics/Collision/Resolve/Constraints.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Jump.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

void Mlib::collide_grind_infos(
    const PhysicsEngineConfig& cfg,
    std::list<std::unique_ptr<ContactInfo>>& contact_infos,
    const std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos)
{
    for (const auto& [rb, p] : grind_infos) {
        rb->grind_state_.grind_pv_ = dot1d(rb->rbi_.rbp_.rotation_.T(), p.rail_direction.casted<float>());
        if (std::abs(rb->grind_state_.grind_pv_(0)) > std::abs(rb->grind_state_.grind_pv_(2))) {
            rb->grind_state_.grind_axis_ = 0;
        } else {
            rb->grind_state_.grind_axis_ = 2;
        }
        if (rb->jump_state_.wants_to_jump_oversampled_) {
            auto& o0 = *p.rail_rb;
            auto& o1 = *rb;
            auto point_dir = o1.rbi_.rbp_.rotation_.column(rb->grind_state_.grind_axis_);
            point_dir *= sign(dot0d(point_dir, o1.rbi_.rbp_.v_));
            point_dir -= dot0d(point_dir, p.rail_direction.casted<float>()) * p.rail_direction.casted<float>();
            auto n = -gravity_direction + point_dir * 2.f;
            n /= std::sqrt(sum(squared(n)));
            jump(o0.rbi_.rbp_, o1.rbi_.rbp_, cfg.grind_jump_dv, { .vector = n, .position = p.intersection_point });
        } else if (rb->jump_state_.jumping_counter_ > 30 * cfg.nsubsteps) {
            auto n = cross(p.rail_direction, FixedArray<double, 3>{ 0., 1., 0. });
            double l2 = sum(squared(n));
            if (l2 < 1e-12) {
                THROW_OR_ABORT("Rail normal too small");
            }
            n /= std::sqrt(l2);
            if (p.rail_rb->mass() == INFINITY) {
                if (!rb->align_to_surface_state_.touches_alignment_plane_) {
                    contact_infos.push_back(std::unique_ptr<ContactInfo>(new PlaneContactInfo1{
                        rb->rbi_.rbp_,
                        p.rail_rb->velocity_at_position(p.intersection_point),
                        BoundedPlaneEqualityConstraint{
                            .constraint =
                                PlaneEqualityConstraint{
                                .pec = PointEqualityConstraint{
                                    .p0 = rb->abs_grind_point(),
                                    .p1 = p.intersection_point,
                                    .beta = cfg.plane_equality_beta},
                                .plane_normal = n.casted<float>()},
                            .lambda_min = rb->mass() * cfg.velocity_lambda_min,
                            .lambda_max = -rb->mass() * cfg.velocity_lambda_min
                        }}));
                    PlaneNd<double, 3> plane{
                        cross(n, p.rail_direction),
                        p.intersection_point};
                    contact_infos.push_back(std::unique_ptr<ContactInfo>(new NormalContactInfo1{
                        rb->rbi_.rbp_,
                        BoundedPlaneInequalityConstraint{
                            .constraint = PlaneInequalityConstraint{
                                .normal_impulse = NormalImpulse{.normal = plane.normal},
                                .overlap = -float(dot0d(rb->abs_grind_point(), plane.normal) + plane.intercept)},
                            .lambda_min = rb->mass() * cfg.velocity_lambda_min,
                            .lambda_max = 0},
                        rb->abs_grind_point()}));
                } else {
                    contact_infos.push_back(std::unique_ptr<ContactInfo>(new LineContactInfo1<1>{
                        rb->rbi_.rbp_,
                        p.rail_rb->velocity_at_position(p.intersection_point),
                        LineEqualityConstraint<1>{
                            .pec = PointEqualityConstraint{
                                .p0 = rb->abs_grind_point(),
                                .p1 = p.intersection_point,
                                .beta = cfg.point_equality_beta},
                            .null_space = p.rail_direction.casted<float>()}}));
                }
            } else {
                if (!rb->align_to_surface_state_.touches_alignment_plane_) {
                    contact_infos.push_back(std::unique_ptr<ContactInfo>(new PlaneContactInfo2{
                        rb->rbi_.rbp_,
                        p.rail_rb->rbi_.rbp_,
                        BoundedPlaneEqualityConstraint{
                            .constraint =
                                PlaneEqualityConstraint{
                                    .pec = PointEqualityConstraint{
                                        .p0 = rb->abs_grind_point(),
                                        .p1 = p.intersection_point,
                                        .beta = cfg.plane_equality_beta},
                                    .plane_normal = n.casted<float>()},
                            .lambda_min = (rb->mass() * p.rail_rb->mass()) / (rb->mass() + p.rail_rb->mass()) * cfg.velocity_lambda_min,
                            .lambda_max = -(rb->mass() * p.rail_rb->mass()) / (rb->mass() + p.rail_rb->mass()) * cfg.velocity_lambda_min
                        }}));
                    PlaneNd<double, 3> plane{
                        cross(n, p.rail_direction).casted<double>(),
                        p.intersection_point};
                    contact_infos.push_back(std::unique_ptr<ContactInfo>(new NormalContactInfo2{
                        rb->rbi_.rbp_,
                        p.rail_rb->rbi_.rbp_,
                        BoundedPlaneInequalityConstraint{
                            .constraint = PlaneInequalityConstraint{
                                .normal_impulse = NormalImpulse{.normal = plane.normal},
                                .overlap = -float(dot0d(rb->abs_grind_point(), plane.normal) + plane.intercept)},
                            .lambda_min = rb->mass() * cfg.velocity_lambda_min,
                            .lambda_max = 0},
                        rb->abs_grind_point()}));
                } else {
                    contact_infos.push_back(std::unique_ptr<ContactInfo>(new LineContactInfo2<1>{
                        rb->rbi_.rbp_,
                        p.rail_rb->rbi_.rbp_,
                        LineEqualityConstraint<1>{
                            .pec = PointEqualityConstraint{
                                .p0 = rb->abs_grind_point(),
                                .p1 = p.intersection_point,
                                .beta = cfg.point_equality_beta},
                            .null_space = p.rail_direction.casted<float>()}}));
                }
            }
            rb->grind_state_.grinding_ = true;
            rb->grind_state_.grind_direction_ = p.rail_direction.casted<float>();
        }
    }
}
