#include "Avatar_Animation_Updater.hpp"
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

AvatarAnimationUpdater::AvatarAnimationUpdater(
    const RigidBodyVehicle& rb,
    SceneNode& gun_node,
    const std::string& resource_wo_gun,
    const std::string& resource_w_gun)
: rb_{ rb },
  gun_node_{ gun_node },
  resource_wo_gun_{ resource_wo_gun },
  resource_w_gun_{ resource_w_gun },
  surface_power_{ 0.f }
{}

void AvatarAnimationUpdater::notify_movement_intent() {
    auto it = rb_.engines_.find("legs");
    if (it == rb_.engines_.end()) {
        throw std::runtime_error("AvatarAnimationUpdater could not find \"legs\" engine");
    }
    surface_power_ = it->second.surface_power();
}

void AvatarAnimationUpdater::update_animation_state(AnimationState* animation_state) {
    Gun* gun;
    try {
        gun = dynamic_cast<Gun*>(&gun_node_.get_absolute_observer());
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("AvatarAnimationUpdater could not get absolute observer of gun node: " + std::string(e.what()));
    }
    if (gun == nullptr) {
        throw std::runtime_error("AvatarAnimationUpdater received absolute observer that is not a gun");
    }
    std::string resource_name = gun->is_none_gun()
        ? resource_wo_gun_
        : resource_w_gun_;
    std::string new_animation;
    if (gun->is_none_gun()) {
        new_animation = resource_name + ".walking";
    } else {
        if (std::isnan(surface_power_) || (surface_power_ == 0)) {
            new_animation = resource_name + ".idle";
        } else {
            if (sum(abs(rb_.tires_z_ - FixedArray<float, 3>{ -1, 0.f, 0.f })) < 1e-12) {
                new_animation = resource_name + ".strafe_left";
            } else if (sum(abs(rb_.tires_z_ - FixedArray<float, 3>{ 1, 0.f, 0.f })) < 1e-12) {
                new_animation = resource_name + ".strafe_right";
            } else if (rb_.tires_z_(2) < 0) {
                new_animation = resource_name + ".run_forward";
            } else {
                new_animation = resource_name + ".run_backward";
            }
        }
    }
    if (new_animation != animation_state->periodic_skelletal_animation_name) {
        animation_state->periodic_skelletal_animation_name = new_animation;
        animation_state->periodic_skelletal_animation_frame.frame.time = 0.f;
        animation_state->periodic_skelletal_animation_frame.frame.begin = 0.f;
        animation_state->periodic_skelletal_animation_frame.frame.end =
            RenderingContextStack::primary_rendering_resources()->
                scene_node_resources().
                get_animation_duration(animation_state->periodic_skelletal_animation_name);
    }
}
