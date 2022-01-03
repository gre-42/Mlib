#include "Avatar_Animation_Updater.hpp"
#include <Mlib/Physics/Misc/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Vehicle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Style.hpp>

using namespace Mlib;

AvatarAnimationUpdater::AvatarAnimationUpdater(
    const RigidBodyVehicle& rb,
    const std::string& resource_name)
: rb_{ rb },
  resource_name_{ resource_name },
  surface_power_{ 0.f }
{}

void AvatarAnimationUpdater::notify_movement_intent() {
    auto it = rb_.engines_.find("main");
    if (it == rb_.engines_.end()) {
        throw std::runtime_error("AvatarAnimationUpdater could not find main engine");
    }
    surface_power_ = it->second.surface_power();
}

void AvatarAnimationUpdater::update_style(Style* style) {
    std::string new_animation;
    if (std::isnan(surface_power_) || (surface_power_ == 0)) {
        new_animation = resource_name_ + ".idle";
    } else {
        if (sum(abs(rb_.tires_z_ - FixedArray<float, 3>{ -1, 0.f, 0.f })) < 1e-12) {
            new_animation = resource_name_ + ".strafe_left";
        } else if (sum(abs(rb_.tires_z_ - FixedArray<float, 3>{ 1, 0.f, 0.f })) < 1e-12) {
            new_animation = resource_name_ + ".strafe_right";
        } else {
            new_animation = resource_name_ + ".run";
        }
    }
    if (new_animation != style->skelletal_animation_name) {
        style->skelletal_animation_name = new_animation;
        style->skelletal_animation_frame.time = 0.f;
        style->skelletal_animation_frame.end =
            RenderingContextStack::primary_rendering_resources()->
                scene_node_resources().
                get_animation_duration(style->skelletal_animation_name);
    }
}
