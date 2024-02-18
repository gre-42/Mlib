#include "Particle_Resources.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Instantiator.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

ParticleResources::ParticleResources()
    : instance_creators_{ "Instance creator" }
    , instantiators_{ "Instantiator" }
    , instantiator_to_instance_{ "Instantiator to instance" }
{}

ParticleResources::~ParticleResources() = default;

void ParticleResources::insert_instantiator_to_instance(
    std::string instantiator,
    std::string instance)
{
    instantiator_to_instance_.emplace(std::move(instantiator), std::move(instance));
}

std::string ParticleResources::get_instance_for_instantiator(const std::string& instantiator) const {
    return instantiator_to_instance_.get(instantiator);
}

void ParticleResources::insert_instance_creator(
    std::string name,
    std::function<std::shared_ptr<ParticlesInstance>()> instance_creator)
{
    instance_creators_.emplace(std::move(name), std::move(instance_creator));
}

std::shared_ptr<ParticlesInstance> ParticleResources::instantiate_particles_instance(
    const std::string& name) const
{
    return instance_creators_.get(name)();
}

void ParticleResources::insert_instantiator_creator(
    std::string name,
    std::function<std::unique_ptr<IParticleInstantiator>(ParticlesInstance&)> instantiator_creator)
{
    instantiators_.emplace(std::move(name), std::move(instantiator_creator));
}

std::unique_ptr<IParticleInstantiator> ParticleResources::instantiate_particle_instantiator(
    const std::string& name,
    ParticlesInstance& particles_instance) const
{
    return instantiators_.get(name)(particles_instance);
}
