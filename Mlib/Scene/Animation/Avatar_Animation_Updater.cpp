#include "Avatar_Animation_Updater.hpp"
#include <Mlib/Components/Gun.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Interfaces/IDamageable.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

AvatarAnimationUpdater::AvatarAnimationUpdater(
    const RigidBodyVehicle& rb,
    DanglingRef<SceneNode> gun_node,
    const std::string& resource_wo_gun,
    const std::string& resource_w_gun)
    : rb_{ rb }
    , gun_node_{ gun_node.ptr() }
    , resource_wo_gun_{ resource_wo_gun }
    , resource_w_gun_{ resource_w_gun }
    , surface_power_{ 0.f }
    , gun_node_on_destroy_{ gun_node->on_destroy, CURRENT_SOURCE_LOCATION }
{
    gun_node_on_destroy_.add([this](){
        gun_node_ = nullptr;
    }, CURRENT_SOURCE_LOCATION);
}

AvatarAnimationUpdater::~AvatarAnimationUpdater() = default;

static const auto legs_name = VariableAndHash<std::string>{ "legs" };

void AvatarAnimationUpdater::notify_movement_intent() {
    surface_power_ = rb_.engines_.get(legs_name).surface_power();
}

std::unique_ptr<AnimationState> AvatarAnimationUpdater::update_animation_state(
    const AnimationState& animation_state)
{
    Gun& gun = get_gun(*gun_node_);
    std::string resource_name = gun.is_none_gun()
        ? resource_wo_gun_
        : resource_w_gun_;
    if ((rb_.damageable_ != nullptr) && (rb_.damageable_->health() <= 0.f)) {
        auto new_animation = resource_name + ".die";
        if (new_animation != animation_state.aperiodic_skelletal_animation_name) {
            return std::unique_ptr<AnimationState>(new AnimationState{
                .periodic_skelletal_animation_name = animation_state.periodic_skelletal_animation_name,
                .aperiodic_skelletal_animation_name = new_animation,
                .periodic_skelletal_animation_frame = animation_state.periodic_skelletal_animation_frame,
                .aperiodic_animation_frame = AnimationFrame{
                    .begin = 0.f,
                    .end = RenderingContextStack::primary_scene_node_resources()
                        .get_animation_duration(new_animation),
                    .time = 0.f},
                .delete_node_when_aperiodic_animation_finished = true});
        }
    } else {
        std::string new_animation;
        if (gun.is_none_gun()) {
            new_animation = resource_name + ".walking";
        } else {
            if (std::isnan(surface_power_) || (surface_power_ == 0)) {
                auto v = dot(rb_.rbp_.v_, rb_.rbp_.rotation_);
                if (std::abs(v(0)) > std::abs(v(2))) {
                    if (v(0) <= -1.f * kph) {
                        new_animation = resource_name + ".strafe_left";
                    } else if (v(0) >= 1.f * kph) {
                        new_animation = resource_name + ".strafe_right";
                    }
                } else {
                    if (v(2) <= -1.f * kph) {
                        new_animation = resource_name + ".run_forward";
                    } else if (v(2) >= 1.f * kph) {
                        new_animation = resource_name + ".run_backward";
                    }
                }
                if (new_animation.empty()) {
                    new_animation = resource_name + ".idle";
                }
            } else {
                if (sum(abs(rb_.tires_z_ - FixedArray<float, 3>{ -1.f, 0.f, 0.f })) < 1e-12) {
                    new_animation = resource_name + ".strafe_left";
                } else if (sum(abs(rb_.tires_z_ - FixedArray<float, 3>{ 1.f, 0.f, 0.f })) < 1e-12) {
                    new_animation = resource_name + ".strafe_right";
                } else if (rb_.tires_z_(2) < 0) {
                    new_animation = resource_name + ".run_forward";
                } else {
                    new_animation = resource_name + ".run_backward";
                }
            }
        }
        if (new_animation != animation_state.periodic_skelletal_animation_name) {
            return std::unique_ptr<AnimationState>(new AnimationState{
                .periodic_skelletal_animation_name = new_animation,
                .aperiodic_skelletal_animation_name = animation_state.aperiodic_skelletal_animation_name,
                .periodic_skelletal_animation_frame = AnimationFrame{
                    .begin = 0.f,
                    .end = RenderingContextStack::primary_scene_node_resources()
                        .get_animation_duration(new_animation),
                    .time = 0.f},
                .aperiodic_animation_frame = animation_state.aperiodic_animation_frame});
        }
    }
    return nullptr;
}
