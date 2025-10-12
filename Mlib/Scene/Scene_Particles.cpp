#include "Scene_Particles.hpp"
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Material/Particle_Type.hpp>
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
    RigidBodies& rigid_bodies,
    const VariableAndHash<std::string>& node_name,
    ParticleType particle_type)
    : particle_renderer{ std::make_shared<ParticleRenderer>(particle_resources, particle_type) }
    , smoke_particle_generator{ rendering_resources, scene_node_resources, particle_renderer, scene, rigid_bodies }
{
    switch (particle_type) {
        case ParticleType::NONE:
            THROW_OR_ABORT("Particle type \"none\" does not require scene particles");
        case ParticleType::SMOKE:
        {
            if (node_name->empty()) {
                THROW_OR_ABORT("Smoke particles require a node name");
            }
            auto node = make_unique_scene_node(PoseInterpolationMode::DISABLED);
            node->add_renderable(
                VariableAndHash<std::string>{ "particles" },
                particle_renderer);
            scene.add_root_node(
                node_name,
                std::move(node),
                RenderingDynamics::STATIC,
                RenderingStrategies::OBJECT);
            return;
        }
        case ParticleType::SKIDMARK:
        case ParticleType::SEA_SPRAY:
            if (!node_name->empty()) {
                THROW_OR_ABORT("Skidmark and sea spray particles do not require a node name");
            }
            return;
        case ParticleType::WATER_WAVE:
            THROW_OR_ABORT("Water waves do not require scene particles");
    }
    THROW_OR_ABORT("Unknown particle type");
}

SceneParticles::~SceneParticles() = default;
