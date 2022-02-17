#include "Skater_Animation_Updater.hpp"
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Style.hpp>

using namespace Mlib;

SkaterAnimationUpdater::SkaterAnimationUpdater(
    const RigidBodyVehicle& rb,
    const std::string& resource)
: rb_{ rb },
  resource_{ resource }
{}

void SkaterAnimationUpdater::notify_movement_intent()
{}

void SkaterAnimationUpdater::update_style(Style* style) {
    std::string new_animation;
    if (rb_.revert_surface_power_) {
        new_animation = resource_ + ".left";
    } else {
        new_animation = resource_ + ".right";
    }
    if (new_animation != style->skelletal_animation_name) {
        style->skelletal_animation_name = new_animation;
        style->skelletal_animation_frame.time = 0.f;
        style->skelletal_animation_frame.begin = 0.f;
        style->skelletal_animation_frame.end =
            RenderingContextStack::primary_rendering_resources()->
                scene_node_resources().
                get_animation_duration(style->skelletal_animation_name);
    }
}
