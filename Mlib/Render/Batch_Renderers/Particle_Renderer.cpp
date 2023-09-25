#include "Particle_Renderer.hpp"
#include <Mlib/Render/Batch_Renderers/Particle_Instantiator.hpp>
#include <Mlib/Render/Batch_Renderers/Particles_Instance.hpp>
#include <Mlib/Render/Particle_Resources.hpp>
#include <mutex>

using namespace Mlib;

ParticleRenderer::ParticleRenderer(ParticleResources& resources)
: resources_{resources},
  instances_{mutex_, [&resources](const std::string &name) {
    return resources.instantiate_particles_instance(name);
  }},
  instantiators_{mutex_, [this, &resources](const std::string& name){
    return resources.instantiate_particle_instantiator(
        name,
        *instances_.get(resources.get_instance_for_instantiator(name)));
  }}
{}

ParticleRenderer::~ParticleRenderer() = default;

IParticleInstantiator &ParticleRenderer::get_instantiator(const std::string &name) {
    return *instantiators_.get(name);
}

void ParticleRenderer::preload(const std::string &name) {
    instances_.get(resources_.get_instance_for_instantiator(name))->preload();
}

void ParticleRenderer::move(float dt) {
    for (auto& [_, instance] : instances_) {
        instance->move(dt);
    }
}

void ParticleRenderer::render(
    const FixedArray<double, 4, 4>& vp,
    const TransformationMatrix<float, double, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const ExternalRenderPass& external_render_pass) const
{
    std::shared_lock lock{mutex_};
    for (const auto& [_, instance] : instances_) {
        instance->render(
            vp,
            iv,
            lights,
            scene_graph_config,
            render_config,
            external_render_pass);
    }
}
