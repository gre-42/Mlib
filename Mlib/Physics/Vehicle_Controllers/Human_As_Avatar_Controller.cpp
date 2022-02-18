#include "Human_As_Avatar_Controller.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Style_Updater.hpp>
#include <stdexcept>

using namespace Mlib;

HumanAsAvatarController::HumanAsAvatarController(SceneNode& node)
{
    rb_ = dynamic_cast<RigidBodyVehicle*>(node.get_absolute_movable());
    if (rb_ == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body vehicle");
    }
    ypln_ = dynamic_cast<YawPitchLookAtNodes*>(node.get_relative_movable());
    if (ypln_ == nullptr) {
        throw std::runtime_error("Relative movable is not a ypln");
    }
}

HumanAsAvatarController::~HumanAsAvatarController()
{}

void HumanAsAvatarController::apply() {
    if (any(abs(legs_z_) > float(1e-12))) {
        rb_->tires_z_ = legs_z_ / std::sqrt(sum(squared(legs_z_)));
        rb_->set_surface_power("legs", surface_power_);
    } else {
        rb_->tires_z_ = { 0.f, 0.f, 1.f };
        rb_->set_surface_power("legs", NAN);
    }
    if (!std::isnan(target_yaw_)) {
        ypln_->set_yaw(target_yaw_);
    }
    if (!std::isnan(target_pitch_)) {
        ypln_->pitch_look_at_node()->set_pitch(target_pitch_);
    }
    if (!std::isnan(dyaw_)) {
        ypln_->increment_yaw(dyaw_);
    }
    if (!std::isnan(dpitch_)) {
        ypln_->pitch_look_at_node()->increment_pitch(dpitch_);
    }
    if (rb_->style_updater_ != nullptr) {
        rb_->style_updater_->notify_movement_intent();
    }
}
