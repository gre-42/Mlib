#include "Human_As_Avatar_Controller.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Animation_State_Updater.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

HumanAsAvatarController::HumanAsAvatarController(SceneNode& node)
{
    rb_ = dynamic_cast<RigidBodyVehicle*>(&node.get_absolute_movable());
    if (rb_ == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body vehicle");
    }
    ypln_ = dynamic_cast<YawPitchLookAtNodes*>(&node.get_relative_movable());
    if (ypln_ == nullptr) {
        THROW_OR_ABORT("Relative movable is not a ypln");
    }
}

HumanAsAvatarController::~HumanAsAvatarController()
{}

void HumanAsAvatarController::apply() {
    if (any(abs(legs_z_) > float(1e-12))) {
        rb_->tires_z_ = legs_z_ / std::sqrt(sum(squared(legs_z_)));
        rb_->set_surface_power("legs", EnginePowerIntent{.surface_power = surface_power_, .delta_relaxation = 0.f});
    } else {
        rb_->tires_z_ = { 0.f, 0.f, 1.f };
        rb_->set_surface_power("legs", EnginePowerIntent{.surface_power = NAN, .delta_relaxation = 0.f});
    }
    if (!std::isnan(target_yaw_)) {
        ypln_->set_yaw(target_yaw_);
    }
    if (!std::isnan(target_pitch_)) {
        ypln_->pitch_look_at_node().set_pitch(target_pitch_);
    }
    if (!std::isnan(dyaw_)) {
        ypln_->increment_yaw(dyaw_);
    }
    if (!std::isnan(dpitch_)) {
        ypln_->pitch_look_at_node().increment_pitch(dpitch_);
    }
    if (rb_->animation_state_updater_ != nullptr) {
        rb_->animation_state_updater_->notify_movement_intent();
    }
}
