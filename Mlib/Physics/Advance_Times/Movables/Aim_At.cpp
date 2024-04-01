#include "Aim_At.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Aim.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

AimAt::AimAt(
    AdvanceTimes& advance_times,
    DanglingRef<SceneNode> follower_node,
    DanglingRef<SceneNode> gun_node,
    float bullet_start_offset,
    float bullet_velocity,
    bool bullet_feels_gravity,
    float gravity,
    float locked_on_cosine,
    const std::function<float()>& velocity_estimation_error)
    : shutting_down_{ false }
    , absolute_point_to_aim_at_{ NAN }
    , relative_point_to_aim_at_{ NAN }
    , followed_node_{ nullptr }
    , gun_node_{ gun_node.ptr() }
    , advance_times_{ advance_times }
    , follower_{ get_rigid_body_vehicle(follower_node) }
    , followed_{ nullptr }
    , bullet_start_offset_{ bullet_start_offset }
    , bullet_velocity_{ bullet_velocity }
    , bullet_feels_gravity_{ bullet_feels_gravity }
    , gravity_{ gravity }
    , locked_on_cosine_{ locked_on_cosine }
    , target_locked_on_{ false }
    , velocity_estimation_error_{ velocity_estimation_error }
    , gun_node_on_destroy_{ gun_node->on_destroy }
    , follower_node_on_destroy_{ follower_node->on_destroy }
{
    gun_node->set_sticky_absolute_observer(*this);
    dgs_.add([gun_node]() { gun_node->clear_sticky_absolute_observer(); });
    advance_times_.add_advance_time(*this);
    dgs_.add([this]() { advance_times_.delete_advance_time(*this, CURRENT_SOURCE_LOCATION); });
    gun_node_on_destroy_.add([this]() { if (!shutting_down_) { delete this; }});
    follower_node_on_destroy_.add([this]() { if (!shutting_down_) { delete this; }});
}

AimAt::~AimAt() {
    shutting_down_ = true;
}

void AimAt::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) {
    if (followed_ != nullptr) {
        {
            auto bullet_launch_position =
                gun_node_->absolute_model_matrix().t() -
                (bullet_start_offset_ * gun_node_->absolute_model_matrix().R().column(2)).casted<double>();
            auto initial_bullet_velocity = follower_.velocity_at_position(bullet_launch_position);
            float verr = velocity_estimation_error_();
            auto offset = fixed_zeros<double, 3>();
            float t = 0;
            for (size_t i = 0; ; ++i) {
                RigidBodyPulses rbp = followed_->rbp_;
                rbp.v_ -= initial_bullet_velocity;
                rbp.v_ *= (1 + verr);
                rbp.advance_time(t);
                if (i == 10) {
                    absolute_point_to_aim_at_ = offset + rbp.transform_to_world_coordinates(followed_->target_);
                    relative_point_to_aim_at_ = absolute_model_matrix.itransform(absolute_point_to_aim_at_);
                    break;
                }
                Aim aim{
                    absolute_model_matrix.t(),
                    rbp.transform_to_world_coordinates(followed_->target_),
                    bullet_start_offset_,
                    bullet_velocity_,
                    bullet_feels_gravity_ ? gravity_ : 0.f,
                    1e-6,
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
            auto dir = absolute_point_to_aim_at_ - absolute_model_matrix.t();
            auto l2 = sum(squared(dir));
            if (l2 < 1e-12) {
                target_locked_on_ = false;
            } else {
                target_locked_on_ = -dot0d((dir / std::sqrt(l2)).casted<float>(), gun_node_->absolute_model_matrix().R().column(2)) > locked_on_cosine_;
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

void AimAt::set_followed(DanglingPtr<SceneNode> followed_node)
{
    followed_node_on_destroy_.reset();
    followed_node_ = followed_node;
    if (followed_node == nullptr) {
        followed_ = nullptr;
    } else {
        followed_ = &get_rigid_body_vehicle(*followed_node);
        followed_node_on_destroy_.emplace(followed_node_->on_destroy);
        followed_node_on_destroy_.value().add([this]() {
            followed_node_ = nullptr;
            followed_ = nullptr;
            followed_node_on_destroy_.reset();
            });
    }
}

bool AimAt::target_locked_on() const {
    return target_locked_on_;
}

void AimAt::advance_time(float dt, std::chrono::steady_clock::time_point time) {
    // do nothing (yet)
}

void AimAt::set_bullet_velocity(float value) {
    bullet_velocity_ = value;
}

void AimAt::set_bullet_feels_gravity(bool value) {
    bullet_feels_gravity_ = value;
}

const FixedArray<double, 3>& AimAt::absolute_point_to_aim_at() const {
    return absolute_point_to_aim_at_;
}

const FixedArray<double, 3>& AimAt::relative_point_to_aim_at() const {
    return relative_point_to_aim_at_;
}
