#include "Skater_Animation_Updater.hpp"
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Style.hpp>

using namespace Mlib;

SkaterAnimationUpdater::SkaterAnimationUpdater(
    const RigidBodyVehicle& rb,
    SceneNode& skateboard_node,
    const std::string& resource)
: rb_{ rb },
  skateboard_node_{ skateboard_node },
  resource_{ resource }
{}

void SkaterAnimationUpdater::notify_movement_intent()
{}

void SkaterAnimationUpdater::update_style(Style* style) {
    std::string new_skelletal_animation = resource_;
    if (rb_.revert_surface_power_state_.revert_surface_power_) {
        new_skelletal_animation += ".left";
    } else {
        new_skelletal_animation += ".right";
    }
    if (rb_.jump_state_.wants_to_jump_) {
        new_skelletal_animation += ".jump";
        std::string new_skateboard_animation = new_skelletal_animation + ".skateboard";
        style->aperiodic_skelletal_animation_name = new_skelletal_animation;
        style->aperiodic_animation_frame.frame.time = 0.f;
        style->aperiodic_animation_frame.frame.begin = 0.f;
        style->aperiodic_animation_frame.frame.end =
                std::max(
                    RenderingContextStack::primary_rendering_resources()->
                        scene_node_resources().
                        get_animation_duration(new_skelletal_animation),
                    RenderingContextStack::primary_rendering_resources()->
                        scene_node_resources().
                        get_animation_duration(new_skateboard_animation));
        skateboard_node_.set_aperiodic_animation(new_skateboard_animation);
    } else if (new_skelletal_animation != style->periodic_skelletal_animation_name) {
        style->periodic_skelletal_animation_name = new_skelletal_animation;
        style->periodic_skelletal_animation_frame.frame.time = 0.f;
        style->periodic_skelletal_animation_frame.frame.begin = 0.f;
        style->periodic_skelletal_animation_frame.frame.end =
            RenderingContextStack::primary_rendering_resources()->
                scene_node_resources().
                get_animation_duration(new_skelletal_animation);
    }
}
