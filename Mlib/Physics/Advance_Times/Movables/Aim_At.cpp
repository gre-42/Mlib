#include "Aim_At.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Aim.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

AimAt::AimAt(
    AdvanceTimes& advance_times,
    DanglingBaseClassRef<SceneNode> follower_node,
    DanglingBaseClassRef<SceneNode> gun_node,
    float bullet_start_offset,
    float bullet_velocity,
    bool bullet_feels_gravity,
    float gravity,
    float locked_on_cosine,
    std::function<float()> velocity_estimation_error)
    : absolute_point_to_aim_at_((ScenePos)NAN)
    , relative_point_to_aim_at_((ScenePos)NAN)
    , followed_node_{ nullptr }
    , gun_node_{ gun_node.ptr() }
    , follower_{ get_rigid_body_vehicle(follower_node) }
    , followed_{ nullptr }
    , bullet_start_offset_{ bullet_start_offset }
    , bullet_velocity_{ bullet_velocity }
    , bullet_feels_gravity_{ bullet_feels_gravity }
    , gravity_{ gravity }
    , locked_on_cosine_{ locked_on_cosine }
    , target_locked_on_{ false }
    , velocity_estimation_error_{ std::move(velocity_estimation_error) }
    , gun_node_on_destroy_{ gun_node->on_destroy, CURRENT_SOURCE_LOCATION }
    , follower_node_on_destroy_{ follower_node->on_destroy, CURRENT_SOURCE_LOCATION }
    , followed_node_on_destroy_{ nullptr, CURRENT_SOURCE_LOCATION }
{
    gun_node->set_sticky_absolute_observer(*this);
    dgs_.add([gun_node]() { gun_node->clear_sticky_absolute_observer(); });
    advance_times.add_advance_time({ *this, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
    gun_node_on_destroy_.add([this]() { global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
    follower_node_on_destroy_.add([this]() { global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

AimAt::~AimAt() {
    on_destroy.clear();
}

void AimAt::set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) {
    if ((followed_ != nullptr) && !std::isnan(bullet_velocity_)) {
        {
            auto bullet_launch_position =
                gun_node_->absolute_model_matrix().t -
                (bullet_start_offset_ * gun_node_->absolute_model_matrix().R.column(2)).casted<ScenePos>();
            auto initial_bullet_velocity = follower_.velocity_at_position(bullet_launch_position);
            float verr = velocity_estimation_error_();
            auto offset = fixed_zeros<ScenePos, 3>();
            float t = 0;
            for (size_t i = 0; ; ++i) {
                RigidBodyPulses rbp = followed_->rbp_;
                rbp.v_com_ -= initial_bullet_velocity;
                rbp.v_com_ *= (1 + verr);
                rbp.advance_time(t);
                if (i == 10) {
                    absolute_point_to_aim_at_ = offset + rbp.transform_to_world_coordinates(followed_->target_);
                    relative_point_to_aim_at_ = absolute_model_matrix.itransform(absolute_point_to_aim_at_);
                    break;
                }
                Aim aim{
                    absolute_model_matrix.t,
                    rbp.transform_to_world_coordinates(followed_->target_),
                    bullet_start_offset_,
                    bullet_velocity_,
                    bullet_feels_gravity_ ? gravity_ : 0.f,
                    (ScenePos)1e-6,
                    10 };
                if (std::isnan(aim.aim_offset)) {
                    absolute_point_to_aim_at_ = NAN;
                    relative_point_to_aim_at_ = NAN;
                    target_locked_on_ = false;
                    return;
                }
                t = (float)aim.time;
                offset(1) = aim.aim_offset;
            }
        }
        {
            auto dir = absolute_point_to_aim_at_ - absolute_model_matrix.t;
            auto l2 = sum(squared(dir));
            if (l2 < 1e-12) {
                target_locked_on_ = false;
            } else {
                target_locked_on_ = -dot0d((dir / std::sqrt(l2)).casted<float>(), gun_node_->absolute_model_matrix().R.column(2)) > locked_on_cosine_;
            }
        }
    } else {
        absolute_point_to_aim_at_ = NAN;
        relative_point_to_aim_at_ = NAN;
        target_locked_on_ = false;
    }
}

bool AimAt::has_followed() const {
    return followed_ != nullptr;
}

void AimAt::set_followed(DanglingBaseClassPtr<SceneNode> followed_node)
{
    followed_node_on_destroy_.clear();
    followed_node_ = followed_node;
    if (followed_node == nullptr) {
        followed_ = nullptr;
    } else {
        followed_ = &get_rigid_body_vehicle(*followed_node);
        followed_node_on_destroy_.set(followed_node_->on_destroy, CURRENT_SOURCE_LOCATION);
        followed_node_on_destroy_.add([this]() {
            followed_node_ = nullptr;
            followed_ = nullptr;
            followed_node_on_destroy_.clear();
            }, CURRENT_SOURCE_LOCATION);
    }
}

bool AimAt::target_locked_on() const {
    return target_locked_on_;
}

void AimAt::advance_time(float dt, const StaticWorld& world) {
    // do nothing (yet)
}

void AimAt::set_bullet_velocity(float value) {
    bullet_velocity_ = value;
}

void AimAt::set_bullet_feels_gravity(bool value) {
    bullet_feels_gravity_ = value;
}

const FixedArray<ScenePos, 3>& AimAt::absolute_point_to_aim_at() const {
    return absolute_point_to_aim_at_;
}

const FixedArray<ScenePos, 3>& AimAt::relative_point_to_aim_at() const {
    return relative_point_to_aim_at_;
}
