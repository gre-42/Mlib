#include "Aim_At.hpp"
#include <Mlib/Assert.hpp>
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
    const RigidBodyVehicle& follower,
    float bullet_start_offset,
    float bullet_velocity,
    bool bullet_feels_gravity,
    float gravity,
    float locked_on_cosine_min,
    const std::function<float()>& velocity_estimation_error)
    : point_to_aim_at_{ NAN }
    , followed_node_{ nullptr }
    , advance_times_{ advance_times }
    , follower_{ follower }
    , followed_{ nullptr }
    , bullet_start_offset_{ bullet_start_offset }
    , bullet_velocity_{ bullet_velocity }
    , bullet_feels_gravity_{ bullet_feels_gravity }
    , gravity_{ gravity }
    , locked_on_cosine_min_{ locked_on_cosine_min }
    , target_locked_on_{ false }
    , velocity_estimation_error_{ velocity_estimation_error }
{}

AimAt::~AimAt() {
    if (followed_node_ != nullptr) {
        followed_node_->clearing_observers.remove(*this);
    }
}

void AimAt::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) {
    if (followed_ != nullptr) {
        {
            float verr = velocity_estimation_error_();
            auto offset = fixed_zeros<double, 3>();
            float t = 0;
            for (size_t i = 0; ; ++i) {
                RigidBodyPulses rbp = followed_->rbp_;
                rbp.v_ -= follower_.rbp_.v_;
                rbp.v_ *= (1 + verr);
                rbp.advance_time(t);
                if (i == 10) {
                    point_to_aim_at_ = absolute_model_matrix.itransform(
                        offset + rbp.transform_to_world_coordinates(followed_->target_));
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
                    point_to_aim_at_ = NAN;
                    target_locked_on_ = false;
                    return;
                }
                t = (float)aim.time;
                offset(1) = aim.aim_offset;
            }
        }
        {
            auto dir = point_to_aim_at_ - absolute_model_matrix.t();
            auto l2 = sum(squared(dir));
            if (l2 < 1e-12) {
                target_locked_on_ = false;
            } else {
                target_locked_on_ = std::abs(dot0d((dir / std::sqrt(l2)).casted<float>(), follower_.rbp_.abs_z())) < locked_on_cosine_min_;
            }
        }
    } else {
        point_to_aim_at_ = NAN;
        target_locked_on_ = false;
    }
}

bool AimAt::has_followed() const {
    return followed_ != nullptr;
}

void AimAt::set_followed(
    DanglingPtr<SceneNode> followed_node,
    const RigidBodyVehicle* followed)
{
    assert_true((followed_node == nullptr) == (followed == nullptr));
    if (followed_node_ != nullptr) {
        followed_node_->clearing_observers.remove(*this);
    }
    followed_node_ = followed_node;
    followed_ = followed;
    if (followed_node != nullptr) {
        followed_node->clearing_observers.add(*this);
    }
}

bool AimAt::target_locked_on() const {
    return target_locked_on_;
}

void AimAt::notify_destroyed(DanglingRef<const SceneNode> destroyed_object) {
    if (destroyed_object.ptr() == followed_node_) {
        followed_node_ = nullptr;
        followed_ = nullptr;
    } else {
        advance_times_.schedule_delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
    }
}

void AimAt::advance_time(float dt) {
    // do nothing (yet)
}

void AimAt::set_bullet_velocity(float value) {
    bullet_velocity_ = value;
}

void AimAt::set_bullet_feels_gravity(bool value) {
    bullet_feels_gravity_ = value;
}

const FixedArray<double, 3>& AimAt::point_to_aim_at() const {
    return point_to_aim_at_;
}
