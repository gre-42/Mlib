#include "Smoke_Particle_Generator.hpp"
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Creator.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

SmokeParticleGenerator::SmokeParticleGenerator(
    RenderingResources& rendering_resources,
    SceneNodeResources& scene_node_resources,
    std::shared_ptr<IParticleRenderer> particle_renderer,
    Scene& scene)
    : rendering_resources_{ rendering_resources }
    , scene_node_resources_{ scene_node_resources }
    , particle_renderer_{ std::move(particle_renderer) }
    , scene_{ scene }
{}

void SmokeParticleGenerator::generate_root(
    const VariableAndHash<std::string>& resource_name,
    const VariableAndHash<std::string>& node_name,
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<float, 3>& rotation,
    const FixedArray<float, 3>& velocity,
    float air_resistance,
    float animation_duration,
    ParticleContainer particle_container,
    const StaticWorld& static_world)
{
    if (particle_container == ParticleContainer::NODE) {
        generate_root_node(
            resource_name,
            node_name,
            position,
            rotation,
            velocity,
            air_resistance,
            animation_duration,
            static_world);
    } else if (particle_container == ParticleContainer::INSTANCE) {
        generate_instance(
            resource_name,
            position,
            rotation,
            velocity,
            air_resistance);
    } else {
        THROW_OR_ABORT("Unknown particle type");
    }
}

void SmokeParticleGenerator::generate_instance(
    const VariableAndHash<std::string>& resource_name,
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<float, 3>& rotation,
    const FixedArray<float, 3>& velocity,
    float air_resistance)
{
    particle_renderer_->get_instantiator(resource_name).add_particle(
        TransformationMatrix<float, ScenePos, 3>{
            tait_bryan_angles_2_matrix(rotation),
            position},
        velocity,
        air_resistance);
}

void SmokeParticleGenerator::generate_root_node(
    const VariableAndHash<std::string>& resource_name,
    const VariableAndHash<std::string>& node_name,
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<float, 3>& rotation,
    const FixedArray<float, 3>& velocity,
    float air_resistance,
    float animation_duration,
    const StaticWorld& static_world)
{
    auto node = make_unique_scene_node(
        position,
        rotation,
        1.f,
        PoseInterpolationMode::DISABLED);
    node->set_animation_state(
        std::unique_ptr<AnimationState>(new AnimationState{
            .reference_time = AperiodicReferenceTime{
                static_world.time,
                std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                    std::chrono::duration<float>(animation_duration / seconds))},
            .delete_node_when_aperiodic_animation_finished = true}),
        AnimationStateAlreadyExistsBehavior::THROW);
    scene_node_resources_.instantiate_child_renderable(
        resource_name,
        ChildInstantiationOptions{
            .rendering_resources = &rendering_resources_,
            .instance_name = resource_name,
            .scene_node = node.ref(DP_LOC),
            .interpolation_mode = PoseInterpolationMode::DISABLED,
            .renderable_resource_filter = RenderableResourceFilter{}});
    scene_.auto_add_root_node(node_name, std::move(node), RenderingDynamics::MOVING);
}

void SmokeParticleGenerator::generate_child_node(
    DanglingRef<SceneNode> parent,
    const VariableAndHash<std::string>& resource_name,
    const VariableAndHash<std::string>& child_node_name,
    const FixedArray<ScenePos, 3>& relative_position,
    float animation_duration)
{
    auto child_node = make_unique_scene_node(
        relative_position,
        fixed_zeros<float, 3>(),
        1.f,
        PoseInterpolationMode::DISABLED);
    child_node->set_animation_state(
        std::unique_ptr<AnimationState>(new AnimationState{
            .aperiodic_animation_frame = AperiodicAnimationFrame{
                AnimationFrame{
                    .begin = 0.f,
                    .end = animation_duration,
                    .time = 0.f}},
            .delete_node_when_aperiodic_animation_finished = true}),
        AnimationStateAlreadyExistsBehavior::THROW);
    scene_node_resources_.instantiate_child_renderable(
        resource_name,
        ChildInstantiationOptions{
            .rendering_resources = &rendering_resources_,
            .instance_name = resource_name,
            .scene_node = child_node.ref(DP_LOC),
            .interpolation_mode = PoseInterpolationMode::DISABLED,
            .renderable_resource_filter = RenderableResourceFilter{}});
    DanglingRef<SceneNode> child_node_ref = child_node.ref(DP_LOC);
    parent->add_child(child_node_name, std::move(child_node), ChildRegistrationState::REGISTERED);
    scene_.register_node(child_node_name, child_node_ref);
}

std::string SmokeParticleGenerator::generate_suffix() {
    return scene_.get_temporary_instance_suffix();
}
