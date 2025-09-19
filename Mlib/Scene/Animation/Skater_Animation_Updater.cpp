#include "Skater_Animation_Updater.hpp"
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

static const auto NO_ANIMATION = VariableAndHash<std::string>{"<no_animation>"};

SkaterAnimationUpdater::SkaterAnimationUpdater(
    const RigidBodyVehicle& rb,
    DanglingBaseClassRef<SceneNode> skateboard_node,
    std::string resource)
    : rb_{ rb }
    , skateboard_node_{ skateboard_node.ptr() }
    , resource_{ std::move(resource) }
    , skateboard_node_on_destroy_{ skateboard_node->on_destroy, CURRENT_SOURCE_LOCATION }
{
    skateboard_node_->set_periodic_animation(NO_ANIMATION);
    skateboard_node_on_destroy_.add([this](){
        skateboard_node_ = nullptr;
    }, CURRENT_SOURCE_LOCATION);
}

void SkaterAnimationUpdater::notify_movement_intent()
{}

std::unique_ptr<AnimationState> SkaterAnimationUpdater::update_animation_state(
    const AnimationState& animation_state)
{
    std::string new_skelletal_anim = resource_;
    if (rb_.revert_surface_power_state_.revert_surface_power_) {
        new_skelletal_anim += ".left";
    } else {
        new_skelletal_anim += ".right";
    }
    if (rb_.jump_state_.wants_to_jump_) {
        new_skelletal_anim += ".jump";
        auto new_skateboard_animation = VariableAndHash<std::string>{new_skelletal_anim + ".skateboard"};
        auto new_skelletal_animation = VariableAndHash<std::string>{new_skelletal_anim};
        skateboard_node_->set_aperiodic_animation(new_skateboard_animation);
        return std::unique_ptr<AnimationState>{ new AnimationState{
            .periodic_skelletal_animation_name = animation_state.periodic_skelletal_animation_name,
            .aperiodic_skelletal_animation_name = new_skelletal_animation,
            .periodic_skelletal_animation_frame = animation_state.periodic_skelletal_animation_frame,
            .aperiodic_animation_frame = AnimationFrame{
                .begin = 0.f,
                .end = std::max(
                    RenderingContextStack::primary_scene_node_resources()
                        .get_animation_duration(new_skelletal_animation),
                    RenderingContextStack::primary_scene_node_resources()
                        .get_animation_duration(new_skateboard_animation)),
                .time = 0.f} }};
    } else {
        auto new_skelletal_animation = VariableAndHash<std::string>{new_skelletal_anim};
        if (new_skelletal_animation != animation_state.periodic_skelletal_animation_name) {
            return std::unique_ptr<AnimationState>{ new AnimationState{
                .periodic_skelletal_animation_name = new_skelletal_animation,
                .aperiodic_skelletal_animation_name = animation_state.aperiodic_skelletal_animation_name,
                .periodic_skelletal_animation_frame = AnimationFrame{
                    .begin = 0.f,
                    .end = RenderingContextStack::primary_scene_node_resources()
                        .get_animation_duration(new_skelletal_animation),
                    .time = 0.f},
                .aperiodic_animation_frame = animation_state.aperiodic_animation_frame}};
        }
    }
    return nullptr;
}
