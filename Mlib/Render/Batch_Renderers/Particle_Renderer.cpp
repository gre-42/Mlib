#include "Particle_Renderer.hpp"
#include <Mlib/Render/Batch_Renderers/Particle_Creator.hpp>
#include <Mlib/Render/Batch_Renderers/Particles_Instance.hpp>
#include <Mlib/Render/Resource_Managers/Particle_Resources.hpp>
#include <mutex>

using namespace Mlib;

ParticleRenderer::ParticleRenderer(ParticleResources& resources)
    : resources_{ resources }
    , instances_{ [&resources](const VariableAndHash<std::string>& name) {
        return resources.instantiate_particles_instance(*name);
      } }
    , instantiators_{ [this, &resources](const VariableAndHash<std::string>& name) {
        return resources.instantiate_particle_creator(
            *name,
            *instances_.get(resources.get_instance_for_creator(*name)));
      } }
{}

ParticleRenderer::~ParticleRenderer() = default;

IParticleCreator& ParticleRenderer::get_instantiator(const VariableAndHash<std::string>& name) {
    return *instantiators_.get(name);
}

void ParticleRenderer::preload(const std::string& name) {
    instances_.get(resources_.get_instance_for_creator(name))->preload();
}

void ParticleRenderer::move(float dt, const StaticWorld& world) {
    for (auto& [_, instance] : instances_.shared()) {
        instance->move(dt, world);
    }
}

void ParticleRenderer::render(
    ParticleSubstrate substrate,
    const FixedArray<ScenePos, 4, 4>& vp,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const ExternalRenderPass& external_render_pass) const
{
    for (const auto& [_, instance] : instances_.shared()) {
        if (instance->substrate() != substrate) {
            continue;
        }
        instance->render(
            vp,
            iv,
            lights,
            skidmarks,
            scene_graph_config,
            render_config,
            external_render_pass);
    }
}
