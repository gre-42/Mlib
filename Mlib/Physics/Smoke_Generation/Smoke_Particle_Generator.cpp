#include "Smoke_Particle_Generator.hpp"
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Instantiator.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

SmokeParticleGenerator::SmokeParticleGenerator(
    RenderingResources* rendering_resources,
    SceneNodeResources& scene_node_resources,
    Scene& scene)
: rendering_resources_{rendering_resources},
  scene_node_resources_{scene_node_resources},
  scene_{scene}
{}

void SmokeParticleGenerator::generate_root(
    const std::string& resource_name,
    const std::string& node_name,
    const FixedArray<double, 3>& position,
    float animation_duration,
    ParticleType particle_type)
{
    if (particle_type == ParticleType::NODE) {
        auto node = make_dunique<SceneNode>();
        node->set_position(position);
        node->set_animation_state(std::unique_ptr<AnimationState>(new AnimationState{
            .aperiodic_animation_frame = AperiodicAnimationFrame{
                .frame = AnimationFrame{
                    .begin = 0.f,
                    .end = animation_duration,
                    .time = 0.f}},
            .delete_node_when_aperiodic_animation_finished = true}));
        scene_node_resources_.instantiate_renderable(
            resource_name,
            InstantiationOptions{
                .rendering_resources = rendering_resources_,
                .instance_name = resource_name,
                .scene_node = node.ref(DP_LOC),
                .renderable_resource_filter = RenderableResourceFilter{}});
        scene_.add_root_node(node_name, std::move(node));
    } else if (particle_type == ParticleType::INSTANCE) {
        scene_.particle_instantiator(resource_name).add_particle(
            TransformationMatrix<float, double, 3>{fixed_identity_array<float, 3>(),
            position});
    } else {
        THROW_OR_ABORT("Unknown particle type");
    }
}

void SmokeParticleGenerator::generate_child(
    DanglingRef<SceneNode> parent,
    const std::string& resource_name,
    const std::string& child_node_name,
    const FixedArray<double, 3>& relative_position,
    float animation_duration)
{
    auto child_node = make_dunique<SceneNode>();
    child_node->set_position(relative_position);

    child_node->set_animation_state(std::unique_ptr<AnimationState>(new AnimationState{
        .aperiodic_animation_frame = AperiodicAnimationFrame{
            .frame = AnimationFrame{
                .begin = 0.f,
                .end = animation_duration,
                .time = 0.f}},
        .delete_node_when_aperiodic_animation_finished = true}));
    scene_node_resources_.instantiate_renderable(
        resource_name,
        InstantiationOptions{
            .instance_name = resource_name,
            .scene_node = child_node.ref(DP_LOC),
            .renderable_resource_filter = RenderableResourceFilter{}});
    DanglingRef<SceneNode> child_node_ref = child_node.ref(DP_LOC);
    parent->add_child(child_node_name, std::move(child_node), ChildRegistrationState::REGISTERED);
    scene_.register_node(child_node_name, child_node_ref);
}

std::string SmokeParticleGenerator::generate_suffix() {
    return scene_.get_temporary_instance_suffix();
}
