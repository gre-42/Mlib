#include "Scene_Particles.hpp"
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Render/Batch_Renderers/Particle_Renderer.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>

using namespace Mlib;

SceneParticles::SceneParticles(
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources,
    ParticleResources& particle_resources,
    Scene& scene,
    const VariableAndHash<std::string>& node_name)
    : particle_renderer{ std::make_shared<ParticleRenderer>(particle_resources) }
    , smoke_particle_generator{ rendering_resources, scene_node_resources, *particle_renderer, scene }
{
    auto node = make_unique_scene_node(PoseInterpolationMode::DISABLED);
    node->add_renderable(
        VariableAndHash<std::string>{ "particles" },
        particle_renderer);
    scene.add_root_node(
        node_name,
        std::move(node),
        RenderingDynamics::STATIC,
        RenderingStrategies::OBJECT);
}

SceneParticles::~SceneParticles() = default;
