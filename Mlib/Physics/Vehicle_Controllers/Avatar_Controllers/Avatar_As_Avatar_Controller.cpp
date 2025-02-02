#include "Avatar_As_Avatar_Controller.hpp"
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

AvatarAsAvatarController::AvatarAsAvatarController(
    RigidBodyVehicle& rb,
    YawPitchLookAtNodes& ypln)
    : rb_ { rb }
    , ypln_{ ypln }
{}

AvatarAsAvatarController::~AvatarAsAvatarController()
{}

static const auto legs_name = VariableAndHash<std::string>{ "legs" };

void AvatarAsAvatarController::apply() {
    if ((any(abs(legs_z_) > float(1e-12))) && (drive_relaxation_ > 0.f)) {
        rb_.tires_z_ = legs_z_ / std::sqrt(sum(squared(legs_z_)));
        rb_.set_surface_power(legs_name, EnginePowerIntent{
            .surface_power = surface_power_,
            .drive_relaxation = drive_relaxation_});
    } else {
        rb_.tires_z_ = { 0.f, 0.f, 1.f };
        rb_.set_surface_power(legs_name, EnginePowerIntent{.surface_power = NAN});
    }
    if (!std::isnan(target_yaw_)) {
        ypln_.set_yaw(target_yaw_);
    }
    if (!std::isnan(target_pitch_)) {
        ypln_.pitch_look_at_node().set_pitch(target_pitch_);
    }
    if (!std::isnan(dyaw_)) {
        ypln_.increment_yaw(dyaw_, dyaw_relaxation_);
    }
    if (!std::isnan(dpitch_)) {
        ypln_.pitch_look_at_node().increment_pitch(dpitch_, dpitch_relaxation_);
    }
    if (rb_.animation_state_updater_ != nullptr) {
        rb_.animation_state_updater_->notify_movement_intent();
    }
}
